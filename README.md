## Compiling

tested with: _gcc (Debian 12.2.0-14) 12.2.0_

```
gcc main.c -o smb
```

## Default Values
- host: ::1
- port: 8080
- verbose: false

**Examples (on localhost):**
- smb --app-type=BROKER
- smb --app-type=PUB --topic="My Room/temps"
- smb --app-type=SUB --topic="My Room/#"

The "#" is a wildcard.

## Protocol

### Broker sendet neu erhaltene Messdaten an seine Subscriber
1. Mittels pointer auf den letzten Messwert Verweisen
2. Durch alle Gespeicherten Clienten gehen
3. Wenn es sich um einen Subscriber handelt, topic überprüfen
4. Sollte die topic in gänze, oder teilweise von der Wurzen, übereinstimmen:
   1. Buffer und Pointer an die socket_serialization funktion übergeben
   2. Gefüllten Buffer mit sendto and subscriber senden
