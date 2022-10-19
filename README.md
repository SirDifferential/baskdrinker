![Loathsome Bask Drinker](./bask-drinker.png)

# Loathsome Bask Drinker
## Turn manager for Drunk Souls

Makes sound effects whenever a timer expires to notify the next player is in turn.

# Connectivity

A websocket server can be hosted in localhost. All connections are done in JSON. The "API" to it is below:

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