## Compiling

# Kompilierung

Getestet mit _gcc (Debian 12.2.0-14) 12.2.0_

Entweder mit oder ohne Skript. Das Skript macht aber nichts anderes als die unten gelisteten Befehle.

## Mit Skript
```
chmod +x compile.sh
./compile.sh
```

## Ohne Skript

Verzeichnis für die Binärdateien erstellen.
```
mkdir bin
```

Die einzelnen Programme mit gcc Kompilieren

Der Broker
```
gcc src/smbbroker.c src/smbcommon.c -o bin/smbbroker
```

Der Publisher
```
gcc src/smbpublish.c src/smbcommon.c -o bin/smbpublish
```

Der Subscriber
```
gcc src/smbsubscribe.c src/smbcommon.c -o bin/smbsubscribe
```

Im Folgenden ist von Datum die Rede.
Damit ist **nicht** das Datum im Kalender gemeint, sondern es geht um ein Kompletten Datenpaketinhalt.
Dieser besteht aus einem UNIX Timestamp, einem topic, und Data.

# Funktionsweise

SMB dient dazu Datenquellen, also Publisher, und Datensenken, also Subscriber, sich nicht alle kennen müssen.
Dies erleichtert den Datenaustausch, da alle Anfragen an einen Server gestellt werden.
Um nun noch eine Isolation zu schaffen, sollten die Subscriber in einem eigenen getrennten Netz Laufen, indem sie nur den Broker erreichen.

Die Daten, welche über den SMB ausgetauscht werden bestehen, aus einem Thema und dem eigentlichen Datum.
Jedes Datum untersteht also einem Thema.
Diese Themen in Hierarchisch aufgebaut.

**Funktionsweise des Brokers**

Der Broker ist der Server welcher zwischen Publisher und Subscriber steht.
Dabei ist die Zahl von Publisher oder Subscriber Clients beliebig.

Bekommt der Broker nun eine Anfrage eines Subscriber Clients wird diese gespeichert.
Dazu zählt neben den Verbindungsinformationen auch das abonnierte Thema des Subscribers.
Dies dient dazu, eine Veröffentlichung eines Publishers an für das Thema abonnierte Subscriber zu senden.

**Funktionsweise des Publishers**

Der Publisher erzeugt Daten und sendet diese unter einem spezifischen Thema an den Broker.
Platzhalter Themen sind hier **nicht** erlaubt, da Daten immer einem spezifischen Thema zuordenbar sein müssen.

**Funktionsweise des Subscribers**
Der Subscriber stellt Daten aus den abonnierten Themen bereit, sobald diese vom Broker geliefert werden.
Platzhalter Themen sind hier möglich, dann alle in der Hierarchie untergestellten Themen mit einbezogen und vom Broker an den jeweiligen Subscriber versandt.

# Protokoll

Der Broker wird gestartet und wartet auf sich einwählende Klienten.

Kommt nun ein Datenpaket von einem am Broker an, 
stellt dieser fest, ob es ein Datenpaket von einem Publisher oder Subscriber ist.
Diese Unterscheidung geschieht durch Überprüfung des Datenfeldes. 
Ist dieses leer, so Handelt es sich um einen Subscriber.

Der Broker merkt sich die Verbindungsinformationen und Warte erneut auf eine eingehende Verbindung.
Ist diese nun ein Publisher, das Datenfeld ist also gefüllt worden, werden keine Verbindungsinformationen gespeichert.

Der Broker schaut sich nun das Thema des Datums, an welches von dem Publisher übermittelt wurde.
Dieses Thema wird mit den abonnierten Themen der bekannten Subscriber verglichen und bei einer Übereinstimmung,
wird das Datum an den Subscriber gesendet. Dies wird für alle Subscriber wiederholt. 

Danach geht der Broker in seine Startposition zurück und Warte auf neue Verbindungen.


# Implementierung

Der Standard-port ist **8080** und lässt sich mit **-p x** oder **--port=x** anpassen.
Sonst kann auch, um mehr Informationen zu erhalten, kann **-v** oder **--verbose** genutzt werden.
Dies gilt für alle drei Programme!

Außerdem sinnvoll bei v4 only Systemen ist **-4** oder auch **--legacy_ip**.
Leider können die smb Klienten nicht erkennen, 
ob eine im Hostname notierte ipv6 Adressen auch funktioniert.

Zum Verarbeiten von zusätzlichen options nutze nicht das in glibc enthaltene argp.
Die programme lassen sich immer noch durch "anwendungsname host topic" bedienen.

**Implementierung des Brokers**

Der Broker ist als einziger der Drei programme ipv6 only.
Dies ist notwendig um dual stack über socket zu implementieren.
Dazu wird IPV6_V6ONLY auf 0 gesetzt, es also deaktiviert.
Um ipv4 Klienten zu bedienen ist eine ipv4 immer noch notwendig.
Der Broker bekommt diese allerdings gemappt, also mit einem ::ffff:... Präfix.

Die Endlos schliefe ist wie folgt strukturiert.

Zunächst werden Klient Verbindungen gelesen und mittels der socket_deserialization in einen Struct geladen.
Wenn das Datenfeld leer ist, wird sich ein Subscriber in ein Array gespeichert.
Sonst geht es weiter, um das Thema des Datums mit den themen der bereits bekannten Subscriber abzugleichen.

Gibt es keine bekannten Subscriber wird, die aktuelle iteration beendet.
Sollte kein Subscriber das Thema abboniert haben, ebenso.
Wenn es eine Übereinstimmung gabe, wird das Datum an den endsprechenden Subscriber geschickt.


**Implementierung des Publishers**

Der Publisher bekommt in der main erstmal einige Standard-Werte definiert.
Danach werden die vom Benutzer definierten Werte mittels argp_parse verarbeitet.

Die folgenden Abfragen stellen sicher, dass ein sinnvoller Programmablauf möglich ist.
Im gegensatz zum Subscriber sind Platzhalter in Themen nicht erlaubt, 
weshalb dies ebenfalls gefiltert wird.

Nun werden hostname und topic in init_socket gegeben, 
um die üblichen Schritte durchzuführen, sodass die Sockets einsatzbereit sind. 

Um mit dem Publisher dauerhaft zu senden, muss ein sleep größer 0 gesetzt werden. 

**Implementierung des Subscribers**

Wie der Publisher bekommt auch der Subscriber ein paar Standard-Werte definiert.
Danach werden die vom Benutzer definierten Werte mittels argp_parse verarbeitet.

Im Subscriber muss nur nach einem hostname und nach einem Thema überprüft werden.

Nun werden hostname und topic in init_socket gegeben,
um die üblichen Schritte durchzuführen, sodass die Sockets einsatzbereit sind.
Zusätzlich wird mit getsockname ein zufälliger port gewählt um die Datenpakete in empfang zu nehmen.

Die Endlos schliefe ist wie folgt strukturiert.

Erst werden Daten vom Broker entgegengenommen.
Die Daten werden mit der socket_deserialization in einen Struct geladen.
Die Daten werden nun mittels printf formatiert auf die Console geschrieben.

Der Subscriber wartet nun wieder auf neue Daten.

# Beispiel

Dieses Beispiel veröffentlicht in einem Haus im ersten OG Temperatur und Luftfeuchtigkeit.
Abonniert wird alles im ersten OG.

**Jeder Punkt ist ein eigener Programmaufruf!**

```
smbbroker -v
```

```
smbsubscribe -v localhost "Das Haus/og1/#"
```

```
smbpublisher -v localhost "Das Haus/og1/T"
```

```
smbpublisher localhost "Das Haus/og1/Hm"
```
