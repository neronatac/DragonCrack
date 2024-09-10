import logging


class LoggingMixin:
    def __init__(self, suffix: str = ''):
        self.logger = logging.getLogger('DESCracker.' + self.__class__.__qualname__ + suffix)


def pretty_str_seconds(seconds: int) -> str:
    seconds = int(seconds)
    minutes, seconds = divmod(seconds, 60)
    hours, minutes = divmod(minutes, 60)
    days, hours = divmod(hours, 24)
    years, days = divmod(days, 365)
    if years > 0:
        return '%dy %dd %dh %dm %ds' % (years, days, hours, minutes, seconds)
    elif days > 0:
        return '%dd %dh %dm %ds' % (days, hours, minutes, seconds)
    elif hours > 0:
        return '%dh %dm %ds' % (hours, minutes, seconds)
    elif minutes > 0:
        return '%dm %ds' % (minutes, seconds)
    else:
        return '%ds' % (seconds,)


def des_key_add_parity(key: bytes) -> bytes:
    """
    Adds parity bits to a DES key
    :param key: 56-bit key
    :return: 64-bit key
    """
    return int(''.join([f"{int.from_bytes(key):056b}"[i:i + 7] + '0' for i in range(0, 56, 7)]), base=2).to_bytes(8)
