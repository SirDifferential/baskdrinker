#!/usr/bin/env python

import asyncio
import websockets
import json

async def sendMessage(msgObject, sock, expeptedReply):
    jsstr = None
    try:
        jsstr = json.dumps(msgObject)
    except Exception as e:
        print("Failed JSON encoding message: " + str(e) + ", " + str(msgObject))
        return False

    await sock.send(jsstr)

    resp = await sock.recv()
    try:
        respjs = json.loads(resp)
        if not "msgType" in respjs or respjs["msgType"] != expeptedReply:
            print("did not get appropriate message response: " + str(respjs))
            return False
        print("server ackowledged with " + expeptedReply)
    except Exception as e:
        print("got invalid JSON as reply: " + str(e) + ", " + resp)
        return False
    return True

async def ws_loop():

    async with websockets.connect('ws://localhost:10666') as websocket:

        msg = dict()
        msg["msgType"] = "hello"
        msg["name"] = "baskclient"

        sent = await sendMessage(msg, websocket, "welcome")
        if not sent:
            return

        msg = dict()
        msg["msgType"] = "subscribe"
        msg["topic"] = "warning"
        sent = await sendMessage(msg, websocket, "subscribed")
        if not sent:
            return

        msg = dict()
        msg["msgType"] = "subscribe"
        msg["topic"] = "nextPlayer"
        sent = await sendMessage(msg, websocket, "subscribed")
        if not sent:
            return

        msg = dict()
        msg["msgType"] = "unsubscribe"
        msg["topic"] = "nextPlayer"
        sent = await sendMessage(msg, websocket, "unsubscribed")
        if not sent:
            return

        running = True
        while running:
            resp = await websocket.recv()
            print(resp)

asyncio.get_event_loop().run_until_complete(ws_loop())