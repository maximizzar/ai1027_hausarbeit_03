Compiling:
```
gcc main.c -o smb
```

**Default Values**
- host: ::1
- port: 8080
- verbose: false

**Examples (on localhost):**
- smb --app-type=BROKER
- smb --app-type=PUB --topic="My Room/temps"
- smb --app-type=SUB --topic="My Room/#"

Protocol
