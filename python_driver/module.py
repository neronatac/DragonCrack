from enum import Enum, IntFlag
from socket import socket, AF_INET, SOCK_STREAM

from errors import HWVersionError, DESCrackerError
from utils import LoggingMixin

# version of driver, must match with HW version
VERSION = (0, 1)


class Command(Enum):
    """
    Commands implemented in HW.
    """
    CMD_GENERAL_BASE = 0x00
    CMD_GET_VERSION = 0x01
    CMD_ECHO = 0x02
    CMD_GET_TEMPERATURE = 0x03

    CMD_DES_BASE = 0x10
    CMD_DES_GET_VERSION = 0x11
    CMD_DES_GET_PARAMS = 0x12
    CMD_DES_GET_STATUS = 0x13
    CMD_DES_RESET = 0x14
    CMD_DES_ENABLE = 0x15
    CMD_DES_DISABLE = 0x16
    CMD_DES_SET_PLAINTEXT = 0x17
    CMD_DES_GET_PLAINTEXT = 0x18
    CMD_DES_SET_MASK = 0x19
    CMD_DES_GET_MASK = 0x1A
    CMD_DES_SET_REF = 0x1B
    CMD_DES_GET_REF = 0x1C
    CMD_DES_SET_START_KEY = 0x1D
    CMD_DES_GET_START_KEY = 0x1E
    CMD_DES_SET_END_KEY = 0x1F
    CMD_DES_GET_END_KEY = 0x20
    CMD_DES_GET_CURRENT_KEY = 0x21
    CMD_DES_ENDED = 0x22
    CMD_DES_ENDED_ALL = 0x23
    CMD_DES_RES_AVAILABLE = 0x24
    CMD_DES_RES_AVAILABLE_ALL = 0x25
    CMD_DES_RES_FULL = 0x26
    CMD_DES_RES_FULL_ALL = 0x27
    CMD_DES_GET_RESULT = 0x28


class DESCrackerModuleStatus(IntFlag):
    RESET = 1
    ENABLE = 2


class DESCrackerModule(LoggingMixin):
    """
    Handles a single DESCracker module and connection to HW.
    """
    def __init__(self, ip: str):
        """
        Initializes a module and check its version.
        :param ip: worker's IP address
        """
        LoggingMixin.__init__(self, ip)

        self.logger.debug("Creating instance")

        # min and max temperatures
        self._temp_min: int = -1
        self._temp_max: int = -1
        # number of workers in HW
        self._workers_nbr: int = -1

        self._ip = ip
        self.conn = socket(AF_INET, SOCK_STREAM)

        self._connect()
        self.cmd_des_get_params()

    def __del__(self):
        """
        Closes the connection to HW.
        """
        self.logger.debug("Deleting instance")
        self._disconnect()

    # ######################################################################
    # ### CONNECTION MANAGEMENT
    # ######################################################################
    @property
    def ip(self) -> str:
        return self._ip

    def _connect(self):
        """
        Connects the socket and checks version of HW.
        """
        self.conn.connect((self._ip, 5017))

        ver = self.cmd_get_version()
        if ver != VERSION:
            raise HWVersionError(f"Driver version: {VERSION}; HW version: {ver}")

        self.cmd_echo(b'crack me baby one more time')

        self.logger.info(f"Connected, HW version={ver[0]}.{ver[1]}")

    def _disconnect(self):
        """
        Disables the HW and closes connection.
        :return:
        """
        self.cmd_des_disable()
        self.conn.close()

    # ######################################################################
    # ### COMMANDS
    # ######################################################################
    def _send_recv_cmd(self, cmd: Command, data: bytes | None = None) -> bytes:
        """
        Sends a command and receives the response.
        :param cmd: one of Command enum
        :param data: data to send (optional)
        :return: response data
        """
        # params check
        if data is None:
            data = b''
        if len(data) > 255:
            raise ValueError("Data cannot exceed 255 bytes!")

        if len(data) > 0:
            self.logger.debug(f"Sending command {cmd.name} (data=0x{data.hex(' ', 4)})")
        else:
            self.logger.debug(f"Sending command {cmd.name} (no data)")

        to_send = cmd.value.to_bytes(1, 'big') + len(data).to_bytes(1, 'big') + data
        self.conn.send(to_send)
        resp_len = self.conn.recv(1)[0]
        if resp_len > 0:
            resp = self.conn.recv(resp_len)
            self.logger.debug(f"Received response 0x{resp.hex(' ', 4)}")
        else:
            resp = b''
            self.logger.debug(f"Received empty response")

        return resp

    def cmd_get_version(self) -> tuple:
        """
        Gets version of HW.
        :return: (major, minor)
        """
        resp = self._send_recv_cmd(Command.CMD_GET_VERSION)
        major = int.from_bytes(resp[:4])
        minor = int.from_bytes(resp[4:])
        return major, minor

    def cmd_echo(self, data: bytes):
        """
        Checks if HW echoes valid data.
        :param data: data to echo
        """
        resp = self._send_recv_cmd(Command.CMD_ECHO, data)
        if resp != data:
            raise DESCrackerError(f"Fail during ECHO command ({data} echoed {resp}")

    def cmd_get_temperature(self) -> int:
        """
        Gets temperature of FPGA and updates self.temp_min and self.temp_max
        :return: current temperature
        """
        resp = self._send_recv_cmd(Command.CMD_GET_TEMPERATURE)
        cur = int.from_bytes(resp[:4])
        self._temp_max = int.from_bytes(resp[4:8])
        self._temp_min = int.from_bytes(resp[8:])
        return cur

    def cmd_des_get_version(self) -> tuple:
        """
        Gets version of DES IP.
        :return: (major, minor)
        """
        resp = self._send_recv_cmd(Command.CMD_DES_GET_VERSION)
        major = int.from_bytes(resp[:4])
        minor = int.from_bytes(resp[4:])
        return major, minor

    def cmd_des_get_params(self) -> dict:
        """
        Gets parameters of DES IP (e.g. number of workers) and updates local variables.
        :return: {'name': value}
        """
        resp = self._send_recv_cmd(Command.CMD_DES_GET_PARAMS)
        nbr = int.from_bytes(resp[:4])
        self._workers_nbr = nbr
        return {'workers_nbr': nbr}

    def cmd_des_get_status(self) -> DESCrackerModuleStatus:
        """
        Gets current status of DES IP (enable, reset, etc.).
        :return: status as flag int
        """
        resp = self._send_recv_cmd(Command.CMD_DES_GET_STATUS)
        return DESCrackerModuleStatus(resp[0])

    def cmd_des_reset(self):
        """
        Resets the DES IP.
        Must be done after setting parameters.
        """
        self._send_recv_cmd(Command.CMD_DES_RESET)

    def cmd_des_enable(self):
        """
        Enables the DES IP.
        """
        self._send_recv_cmd(Command.CMD_DES_ENABLE)

    def cmd_des_disable(self):
        """
        Disables the DES IP.
        """
        self._send_recv_cmd(Command.CMD_DES_DISABLE)

    def cmd_des_set_plaintext(self, plaintext: bytes):
        """
        Sets the plaintext of DES workers.
        :param plaintext: plaintext input
        """
        if len(plaintext) != 8:
            raise ValueError(f"DES plaintext must be 8 bytes long ({len(plaintext)} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_PLAINTEXT, plaintext)

    def cmd_des_get_plaintext(self) -> bytes:
        """
        Gets the plaintext of DES workers.
        :return: plaintext input
        """
        return self._send_recv_cmd(Command.CMD_DES_GET_PLAINTEXT)

    def cmd_des_set_mask(self, mask: bytes, nbr: int):
        """
        Sets one mask of DES workers.
        :param mask: mask parameter
        :param nbr: which mask to set (1 or 2)
        """
        if len(mask) != 8:
            raise ValueError(f"Mask must be 8 bytes long ({len(mask)} given)")
        if nbr not in (1, 2):
            raise ValueError(f"Mask number must be 1 or 2 ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_MASK, nbr.to_bytes(1) + mask)

    def cmd_des_get_mask(self, nbr: int) -> bytes:
        """
        Gets one mask of DES workers.
        :param nbr: which mask to get (1 or 2)
        :return: mask parameter
        """
        if nbr not in (1, 2):
            raise ValueError(f"Mask number must be 1 or 2 ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_MASK, nbr.to_bytes(1))

    def cmd_des_set_ref(self, ref: bytes, nbr: int):
        """
        Sets one ref of DES workers.
        :param ref: ref parameter
        :param nbr: which ref to set (1 or 2)
        """
        if len(ref) != 8:
            raise ValueError(f"Ref must be 8 bytes long ({len(ref)} given)")
        if nbr not in (1, 2):
            raise ValueError(f"Mask number must be 1 or 2 ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_REF, nbr.to_bytes(1) + ref)

    def cmd_des_get_ref(self, nbr: int) -> bytes:
        """
        Gets one ref of DES workers.
        :param nbr: which ref to get (1 or 2)
        :return: ref parameter
        """
        if nbr not in (1, 2):
            raise ValueError(f"Ref number must be 1 or 2 ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_REF, nbr.to_bytes(1))

    def cmd_des_set_start_key(self, key: bytes, nbr: int):
        """
        Sets the start key of one DES worker.
        :param key: first key to exhaust
        :param nbr: worker number
        """
        if len(key) != 7:
            raise ValueError(f"Key must be 7 bytes long ({len(key)} given)")
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_START_KEY, nbr.to_bytes(4) + key)

    def cmd_des_get_start_key(self, nbr: int) -> bytes:
        """
        Gets the start key of one DES worker.
        :param nbr: worker number
        :return: start_key parameter
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_START_KEY, nbr.to_bytes(4))

    def cmd_des_set_end_key(self, key: bytes, nbr: int):
        """
        Sets the end key of one DES worker.
        :param key: last key to exhaust
        :param nbr: worker number
        """
        if len(key) != 7:
            raise ValueError(f"Key must be 7 bytes long ({len(key)} given)")
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_END_KEY, nbr.to_bytes(4) + key)

    def cmd_des_get_end_key(self, nbr: int) -> bytes:
        """
        Gets the end key of one DES worker.
        :param nbr: worker number
        :return: end_key parameter
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_END_KEY, nbr.to_bytes(4))

    def cmd_des_get_current_key(self, nbr: int) -> bytes:
        """
        Gets the key currently tested of one DES worker.
        :param nbr: worker number
        :return: current key
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_CURRENT_KEY, nbr.to_bytes(4))

    def cmd_des_ended(self, nbr: int) -> bool:
        """
        Gets the run status of one DES worker (running or finished).
        :param nbr: worker number
        :return: True if worker has finished its work, False if still running
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_ENDED, nbr.to_bytes(4))) == 1

    def cmd_des_ended_all(self) -> int:
        """
        Gets the run status of all DES workers (running or finished).
        :return: integer where each binary digit represents one worker status (1 if finished, 0 if running)
        """
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_ENDED_ALL))

    def cmd_des_result_available(self, nbr: int) -> bool:
        """
        Gets the result status of one DES worker (result available or not).
        :param nbr: worker number
        :return: True if a result is available, False if not
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_RES_AVAILABLE, nbr.to_bytes(4))) == 1

    def cmd_des_result_available_all(self) -> int:
        """
        Gets the result status of all DES workers (result available or not).
        :return: integer where each binary digit represents one result status (1 if available, 0 if empty)
        """
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_RES_AVAILABLE_ALL))

    def cmd_des_result_full(self, nbr: int) -> bool:
        """
        Gets the result memory status of one DES worker (full or not).
        If a worker's result memory is full, it stops working until it is cleared.
        :param nbr: worker number
        :return: True if result memory is full, False if not
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_RES_FULL, nbr.to_bytes(4))) == 1

    def cmd_des_result_full_all(self) -> int:
        """
        Gets the result memory status of all DES workers (full or not).
        If a worker's result memory is full, it stops working until it is cleared.
        :return: integer where each binary digit represents one result status (1 if available, 0 if empty)
        """
        return int.from_bytes(self._send_recv_cmd(Command.CMD_DES_RES_FULL_ALL))

    def cmd_des_get_result(self, nbr: int) -> tuple[int, bytes]:
        """
        Gets one result from one DES worker.
        :param nbr: worker number
        :return: which ref matched and the key that matched
        """
        if not 0 <= nbr < self._workers_nbr:
            raise ValueError(f"Worker number must be in [0,{self._workers_nbr}[ ({nbr} given)")
        resp = self._send_recv_cmd(Command.CMD_DES_GET_RESULT, nbr.to_bytes(4))
        match_nbr = int.from_bytes(resp[:4])
        return match_nbr+1, resp[4:]

    # ######################################################################
    # ### MISC VALUES
    # ######################################################################
    @property
    def temp_max(self) -> int:
        return self._temp_max

    @property
    def temp_min(self) -> int:
        return self._temp_min

    @property
    def workers_nbr(self) -> int:
        return self._workers_nbr
