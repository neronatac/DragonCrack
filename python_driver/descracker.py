import logging
import math
from dataclasses import dataclass
from time import time

from errors import DESCrackerError, DESCrackerTimeoutError
from module import DESCrackerModule
from utils import LoggingMixin, pretty_str_seconds


@dataclass
class Result:
    """
    Result of exhaust
    """
    ref_nbr: int
    key: bytes

    def __str__(self):
        return f"(REF{self.ref_nbr}, 0x{self.key.hex()})"


class DESCracker(LoggingMixin):
    """
    Handles a DESCracker composed of a cluster of modules.
    Each worker is identified by its IP address.
    """

    STATUS_NOT_RUNNING = 0
    STATUS_RUNNING = 1

    def __init__(self, modules_ips: list[str] | None = None):
        """
        Initializes the cluster. Adds some workers if necessary.
        :param modules_ips: list of workers' IPs
        """
        LoggingMixin.__init__(self)

        self.logger.debug("Creating instance")

        # list of workers
        self._modules: list[DESCrackerModule] = []

        if modules_ips is not None:
            self.add_modules(*modules_ips)

    # ######################################################################
    # ### WORKERS MANAGEMENT
    # ######################################################################
    def add_modules(self, *ips: str):
        """
        Adds a module to the DESCracker cluster.
        :param ips: list of workers' IPs
        """
        for ip in ips:
            self.logger.info(f"Adding module: IP={ip}")
            module = DESCrackerModule(ip)
            if len(self._modules) > 0:
                if module.variable_part_width != self._modules[0].variable_part_width:
                    raise DESCrackerError("All modules must have the same variable part width of key")
            self._modules.append(module)

    # ######################################################################
    # ### DES CRACKING
    # ######################################################################
    def exhaust_keys(self,
                     plaintext: bytes,
                     ref1: bytes,
                     mask1: bytes,
                     start: bytes | None = None,
                     end: bytes | None = None,
                     ref2: bytes | None = None,
                     mask2: bytes | None = None,
                     single_worker_module: DESCrackerModule | None = None,
                     single_worker_idx: int | None = None,
                     timeout: float | None = None) -> tuple[list[Result], int, int]:
        """
        Exhaust keys on all workers (blocking until it finishes or it timeouts) and return matches.
        :param plaintext: plaintext to use
        :param ref1: first reference to check
        :param mask1: first mask to apply
        :param start: first key to exhaust (all 0x00 if None)
        :param end: last key to exhaust (all 0xFF if None)
        :param ref2: second reference to check
        :param mask2: second mask to apply
        :param single_worker_module: if defined, does the exhaust only on the first worker of this module
        :param single_worker_idx: index of the worker to work with in module given in previous parameter
        :param timeout: time in seconds at which it raises a DESCrackerTimeoutError (results are in it)
        :return list of results and start and end keys that were actually exhausted
        """
        # param checks
        if len(plaintext) != 8:
            raise ValueError(f"DES plaintext must be 8 bytes long ({len(plaintext)} given)")
        if len(ref1) != 8:
            raise ValueError(f"Ref must be 8 bytes long ({len(ref1)} given)")
        if ref2 is not None and len(ref2) != 8:
            raise ValueError(f"Ref must be 8 bytes long ({len(ref2)} given)")
        if len(mask1) != 8:
            raise ValueError(f"Mask must be 8 bytes long ({len(mask1)} given)")
        if mask2 is not None and len(mask2) != 8:
            raise ValueError(f"Mask must be 8 bytes long ({len(mask2)} given)")
        if start is not None and len(start) != 7:
            raise ValueError(f"Key must be 7 bytes long ({len(start)} given)")
        if end is not None and len(end) != 7:
            raise ValueError(f"Key must be 7 bytes long ({len(end)} given)")
        if single_worker_module is not None and single_worker_module not in self._modules:
            raise ValueError(f"Given module for single worker exhaust is not valid")
        if single_worker_module is not None and single_worker_idx not in range(single_worker_module.workers_nbr):
            raise ValueError(f"Single worker idx must be in [0;{single_worker_module.workers_nbr - 1}] "
                             f"({single_worker_idx} given)")

        # optional params handling
        start = start or bytes(7)
        end = end or bytes([0xFF] * 7)
        ref2 = ref2 or bytes([0xFF] * 8)
        mask2 = mask2 or bytes(8)

        # timer for timeout
        begin_time = time()

        # compute total number of workers
        total_workers = sum([m.workers_nbr for m in self._modules]) if single_worker_module is None else 1

        self.logger.info(f"Launching exhaust from 0x{start.hex(' ', 4)} to 0x{end.hex(' ', 4)}"
                         f" on {total_workers} workers")

        # compute fixed part of keys
        start_fixed = int.from_bytes(start) >> self._modules[0].variable_part_width
        end_fixed = int.from_bytes(end) >> self._modules[0].variable_part_width
        real_start_key = start_fixed << self._modules[0].variable_part_width
        real_end_key = ((end_fixed + 1) << self._modules[0].variable_part_width) - 1

        self.logger.info(
            f"Actually from 0x{real_start_key.to_bytes(7).hex(' ', 4)} "
            f"to 0x{real_end_key.to_bytes(7).hex(' ', 4)}")
        self.logger.info(f"{end_fixed - start_fixed + 1} chunks of {self._modules[0].variable_part_width} bits each")

        # reset modules to clean them then set plaintext, refs and masks
        modules = [single_worker_module] if single_worker_module is not None else self._modules
        for m in modules:
            m.cmd_des_set_plaintext(plaintext)
            assert m.cmd_des_get_plaintext() == plaintext
            m.cmd_des_set_ref(ref1, 1)
            m.cmd_des_set_ref(ref2, 2)
            m.cmd_des_set_mask(mask1, 1)
            m.cmd_des_set_mask(mask2, 2)

        # set keys for each worker in each module, launch and do polling
        results = []
        current_fixed = start_fixed
        first_time = {m: [True] * m.workers_nbr for m in modules}
        workers = [(m, w_idx) for m in modules for w_idx in range(m.workers_nbr)] if single_worker_module is None \
            else [(single_worker_module, single_worker_idx)]
        last_time_1s = time()
        while workers:
            for m, worker_idx in workers:
                if m.cmd_des_result_available(worker_idx) and not first_time[m][worker_idx]:
                    res = Result(*m.cmd_des_get_result(worker_idx))
                    self.logger.debug(f"Result found: {res}")
                    results.append(res)
                elif m.cmd_des_ended(worker_idx) or first_time[m][worker_idx]:
                    first_time[m][worker_idx] = False

                    if current_fixed > end_fixed:  # this worker has finished
                        workers.remove((m, worker_idx))
                        continue

                    m.cmd_des_set_fixed_key(current_fixed.to_bytes(math.ceil(m.fixed_part_width / 8)),
                                            worker_idx)  # set key
                    m.cmd_des_reset(1 << worker_idx)  # launch
                    current_fixed += 1

            # get temperature every second
            if time() - last_time_1s > 1:
                last_time_1s = time()
                for m in modules:
                    self.logger.info(f"Module{m.ip}: {m.cmd_get_temperature()}°C")

            # check timeout
            if timeout and time() - begin_time > timeout:
                raise DESCrackerTimeoutError(f"Timeout occurred during the exhaust!", results)

        return results, real_start_key, real_end_key

    # ######################################################################
    # ### TESTING
    # ######################################################################
    def benchmark(self,
                  number_of_keys: int,
                  all_match: bool = False) -> float:
        """
        Does a dummy bruteforce and returns the number of elapsed seconds.
        :param number_of_keys: number of keys to exhaust
        :param all_match: whether all keys or none should match
        :return: benchmark time
        """
        self.logger.info(f"Launching benchmark {'(all_match)' if all_match else ''} on {number_of_keys} keys")

        last_key = number_of_keys.to_bytes(7)

        plaintext = bytes.fromhex('0001020304050607')
        mask1 = bytes([0x00] * 8)
        ref1 = bytes([0x00] * 8) if all_match else bytes([0xFF] * 8)

        # launch exhaust
        begin_time = time()
        self.logger.debug(f"Benchmark begin time: {begin_time}")
        _, real_start, real_end = self.exhaust_keys(plaintext, ref1, mask1, bytes(7), last_key)

        # get real number of keys
        number_of_keys_real = real_end - real_start

        # it's done!
        elapsed = time() - begin_time
        self.logger.info(
            f"Benchmark {'(all_match)' if all_match else ''} on {number_of_keys_real:_} finished in {elapsed:.02f}s!")
        self.logger.info(f"It corresponds to {int(number_of_keys_real / elapsed):_} keys/s")
        self.logger.info(f"Exhaust on 2**56 would take {pretty_str_seconds(int(2 ** 56 * elapsed / number_of_keys_real))}")
        return elapsed

    def test(self):
        """
        Tests if hardware works well. Raises an Exception if not.
        """
        self.logger.info(f"Beginning of tests")

        for m in self._modules:
            for w in range(m.workers_nbr):
                self.logger.info(f"Beginning of tests on module {m.ip}, worker {w}")
                # test if it stops when valid key is found on REF1 and REF2 then resume and end
                for ref_nbr in (1, 2):
                    self.logger.info(f"Testing single match on REF{ref_nbr}")
                    plaintext = bytes.fromhex('123456ABCD132536')
                    valid_key = bytes.fromhex('ab7420c266f36e')
                    mask = bytes([0xff] * 8)
                    ref = bytes.fromhex('C0B7A8D05F3A829C')
                    start_key = bytes.fromhex('ab7420c266f350')
                    end_key = bytes.fromhex('ab7420c266f380')
                    try:
                        if ref_nbr == 1:
                            results, _, _ = self.exhaust_keys(plaintext,
                                                        ref,
                                                        mask,
                                                        start_key,
                                                        end_key,
                                                        single_worker_module=m,
                                                        single_worker_idx=w,
                                                        timeout=100)
                        else:
                            results, _, _ = self.exhaust_keys(plaintext,
                                                        bytes([0xff] * 8),
                                                        bytes([0x00] * 8),
                                                        start_key,
                                                        end_key,
                                                        ref,
                                                        mask,
                                                        single_worker_module=m,
                                                        single_worker_idx=w,
                                                        timeout=100)
                    except DESCrackerTimeoutError:
                        raise DESCrackerError(f"Module {m.ip} (worker {w}) did not finish within 100s")

                    if len(results) != 1:
                        raise DESCrackerError(f"Module {m.ip} (worker {w}) got {len(results)} results instead of 1")

                    if results[0].ref_nbr != ref_nbr:
                        raise DESCrackerError(f"Expected result on REF{ref_nbr}, got REF{results[0].ref_nbr}")
                    if results[0].key != valid_key:
                        raise DESCrackerError(f"Expected result {valid_key.hex()}, got {results[0].key.hex()}")

                # test consecutive matches during 0.5s
                self.logger.info(f"Testing consecutive matches")
                plaintext = bytes.fromhex('123456ABCD132536')
                mask = bytes([0x00] * 8)
                ref = bytes([0x00] * 8)
                start_key = (0).to_bytes(7)
                end_key = (9).to_bytes(7)
                try:
                    self.exhaust_keys(plaintext,
                                                ref,
                                                mask,
                                                start_key,
                                                end_key,
                                                single_worker_module=m,
                                                single_worker_idx=w,
                                                timeout=0.5)
                except DESCrackerTimeoutError as exc:
                    results = exc.results
                else:
                    raise DESCrackerError(f"Exhaust should have timeout but did not")

                for i in range(len(results)):
                    if results[i].key != i.to_bytes(7):
                        raise DESCrackerError(f"Bad result found for key {i} "
                                              f"(expected {i.to_bytes(7).hex(' ', 4)}, got {results[i].key.hex(' ', 4)})")

            self.logger.info(f"Module {m.ip} works well")

        self.logger.info(f"End of tests, everything is fine")


if __name__ == '__main__':
    # logging stuff
    logger = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    fh = logging.FileHandler('debug_log.log', 'w')
    fh.setLevel(logging.DEBUG)

    sh = logging.StreamHandler()
    sh.setLevel(logging.INFO)

    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    fh.setFormatter(formatter)
    sh.setFormatter(formatter)

    logger.addHandler(fh)
    logger.addHandler(sh)

    cracker = DESCracker()
    cracker.add_modules('192.168.20.42')

    # do tests
    # cracker.test()

    # do benchmarks
    number = 200_000_000_000
    t = cracker.benchmark(number)
    print(f"Benchmark on ~{number:_} done in {t} seconds")

    # number = 10_000
    # t = cracker.benchmark(number, True)
    # print(f"Benchmark (all match) on {number:_} done in {t} seconds")
