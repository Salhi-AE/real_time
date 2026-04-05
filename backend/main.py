from fastapi import FastAPI, WebSocket
import asyncio, json

app = FastAPI()
clients = []

@app.websocket("/ws")
async def ws(ws: WebSocket):
    await ws.accept()
    clients.append(ws)
    try:
        while True:
            await asyncio.sleep(1)
    except:
        if ws in clients:
            clients.remove(ws)

async def broadcaster():
    while True:
        try:
            with open("data.json") as f:
                data = json.load(f)

            for c in clients[:]:
                try:
                    await c.send_json(data)
                except:
                    clients.remove(c)

        except Exception as e:
            print("Error:", e)

        await asyncio.sleep(0.2)

@app.on_event("startup")
async def start():
    asyncio.create_task(broadcaster())