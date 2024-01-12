import asyncio


class Counter:
    """A thread-safe counter."""

    def __init__(self):
        self._value = 0
        self._lock = asyncio.Lock()

    async def increment(self):
        async with self._lock:
            self._value += 1

    def value(self):
        return self._value
