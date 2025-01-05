from enum import Enum
from socket import socket, AF_INET, SOCK_STREAM

from errors import HWVersionError, DESCrackerError
from utils import LoggingMixin

# version of driver, must match with HW version
VERSION = (5, 1)


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
    CMD_DES_SET_PLAINTEXT = 0x13
    CMD_DES_GET_PLAINTEXT = 0x14
    CMD_DES_SET_MASK = 0x15
    CMD_DES_GET_MASK = 0x16
    CMD_DES_SET_REF = 0x17
    CMD_DES_GET_REF = 0x18
    CMD_DES_SET_KEY_RANGE = 0x19
    CMD_DES_GET_KEY_RANGE = 0x1A
    CMD_DES_GET_CURRENT_CHUNK = 0x1B
    CMD_DES_GET_STATUS = 0x1C
    CMD_DES_GET_RESULTS = 0x1D
    CMD_DES_SET_NEW_RESULTS_STATUS = 0x1E
    CMD_DES_GET_NEW_RESULTS_STATUS = 0x1F


class DESStatus(Enum):
    """
    DES Status
    """
    WAITING = 0x01
    RUNNING = 0x02
    FINISHED = 0x03
    ERROR = 0xFF


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
        # variable part width of the key
        self._variable_part_width: int = -1

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
        variable_part_with = int.from_bytes(resp[4:8])
        self._workers_nbr = nbr
        self._variable_part_width = variable_part_with
        return {'workers_nbr': nbr, 'variable_part_with': variable_part_with}

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
        :param nbr: which mask to set (0 or 1)
        """
        if len(mask) != 8:
            raise ValueError(f"Mask must be 8 bytes long ({len(mask)} given)")
        if nbr not in (0, 1):
            raise ValueError(f"Mask number must be 0 or 1 ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_MASK, nbr.to_bytes(1) + mask)

    def cmd_des_get_mask(self, nbr: int) -> bytes:
        """
        Gets one mask of DES workers.
        :param nbr: which mask to get (0 or 1)
        :return: mask parameter
        """
        if nbr not in (0, 1):
            raise ValueError(f"Mask number must be 0 or 1 ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_MASK, nbr.to_bytes(1))

    def cmd_des_set_ref(self, ref: bytes, nbr: int):
        """
        Sets one ref of DES workers.
        :param ref: ref parameter
        :param nbr: which ref to set (0 or 1)
        """
        if len(ref) != 8:
            raise ValueError(f"Ref must be 8 bytes long ({len(ref)} given)")
        if nbr not in (0, 1):
            raise ValueError(f"Mask number must be 0 or 1 ({nbr} given)")

        self._send_recv_cmd(Command.CMD_DES_SET_REF, nbr.to_bytes(1) + ref)

    def cmd_des_get_ref(self, nbr: int) -> bytes:
        """
        Gets one ref of DES workers.
        :param nbr: which ref to get (0 or 1)
        :return: ref parameter
        """
        if nbr not in (0, 1):
            raise ValueError(f"Ref number must be 0 or 1 ({nbr} given)")
        return self._send_recv_cmd(Command.CMD_DES_GET_REF, nbr.to_bytes(1))

    def cmd_des_set_key_range(self, start_key: bytes, end_key: bytes):
        """
        Sets the start and end keys, exhaust between them is automatically launched.
        Variable part of the start and end keys are ignored by the module. Use cmd_des_get_key_range to get real bounds.
        :param start_key: key to start with
        :param end_key: key to end with
        """
        if len(start_key) != 7:
            raise ValueError(f"Keys must be 7 bytes long ({len(start_key)} given)")
        if len(end_key) != 7:
            raise ValueError(f"Keys must be 7 bytes long ({len(end_key)} given)")
        self._send_recv_cmd(Command.CMD_DES_SET_KEY_RANGE, start_key + end_key)

    def cmd_des_get_key_range(self) -> tuple[bytes, bytes]:
        """
        Gets the start and end keys.
        :return: start and end keys
        """
        res = self._send_recv_cmd(Command.CMD_DES_GET_KEY_RANGE)
        return res[:7], res[7:]

    def cmd_des_get_current_chunk(self) -> bytes:
        """
        Gets the current chunk to be exhausted.
        :return: current chunk (only fixed part of the key, not padded)
        """
        return self._send_recv_cmd(Command.CMD_DES_GET_CURRENT_CHUNK)

    def cmd_des_get_status(self) -> DESStatus:
        """
        Gets the status of exhaust: finished or not.
        :return: current status
        """
        return DESStatus(int.from_bytes(self._send_recv_cmd(Command.CMD_DES_GET_STATUS)))

    def cmd_des_get_results(self) -> list[tuple[int, bytes]]:
        """
        Gets some results if any (3 max because of packet size).
        :return: list of tuples: which ref matched and the key that matched
        """
        resp = self._send_recv_cmd(Command.CMD_DES_GET_RESULTS)
        results = []
        for i in range(len(resp) // 8):
            match_nbr = resp[i * 8] >> 7
            key = resp[i * 8 + 1:i * 8 + 8]
            results.append((match_nbr, key))
        return results

    def cmd_des_set_new_results_status(self, get_new_results: bool):
        """
        Enables or disables the recording of new results.
        :param get_new_results: True to enable, False to disable
        """
        self._send_recv_cmd(Command.CMD_DES_SET_NEW_RESULTS_STATUS, int(get_new_results).to_bytes(4))

    def cmd_des_get_new_results_status(self) -> bool:
        """
        Gets the status of new results recording.
        :return: True means enabled, False means disabled
        """
        res = self._send_recv_cmd(Command.CMD_DES_GET_NEW_RESULTS_STATUS)
        return bool(int.from_bytes(res))

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

    @property
    def variable_part_width(self) -> int:
        return self._variable_part_width

    @property
    def fixed_part_width(self) -> int:
        return 56 - self._variable_part_width
