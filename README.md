![Loathsome Bask Drinker](./bask-drinker.png)

# Loathsome Bask Drinker
## Turn manager for Drunk Souls

Software used to control play turns in live gaming sessions with multiple people. This program is especially designed for the From Software catalogue.

The program allows setting turn durations in seconds, and makes sound effects whenever a timer expires to notify the next player is in turn. Also has the option to give an early warning so people don't need to panic.

# Downloads

See [latest release](https://github.com/SirDifferential/baskdrinker/releases)

# Hints

* Ctrl + Alt + Space is a global hotkey for controlling the timer start/stop button
* Hotkeys can have audio loaded for them at the bottom of the window

# Building

* Install a C++ compiler of your choice
* Install [Qt](https://www.qt.io/). You need the following "additional libraries" in the installer:
	* Qt Multimedia
	* Qt Websockets
* Either open baskdrinker.pro in Qt Creator, run qmake, and compile, or execute qmake + make from terminal

# Connectivity

The program can host a websocket server in localhost to send notifications to your client. This can be useful with software such as [Loathsome Pad Swapper](https://github.com/Sonicus/loathsome-pad-swapper) if you wish to dynamically change the gamepad currently sending input to your game of choice.

All communications are done in JSON. The "API" to it is below.

### Hello

Must be sent after opening a connection.

Request:

```
{
	"msgType": "hello",
	"name": "myClientName"
}
```

Response:

```
{
	"msgType": "welcome",
}
```

### Subscribe

Tell the server you wish to receive notifications on a specific event.

Request:

```
{
	"msgType": "subscribe",
	"topic": "validTopicName"
}
```

Valid topics are:

* warning (sent when the warning timer triggers)
* nextPlayer (sent when the timer reaches zero)

Response:

```
{
	"msgType": "subscribed",
}
```

### Unsubscribe

Tell the server you wish to stop receiving notifications on a specific event.

Request:

```
{
	"msgType": "unsubscribe",
	"topic": "validTopicName"
}
```

Response:

```
{
	"msgType": "subscribed",
}
```

### Example client

An example Python implementation of a websocket client can be found in [./baskclient](./baskclient)
