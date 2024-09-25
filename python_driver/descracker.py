import logging
from dataclasses import dataclass
from time import time

from errors import DESCrackerError
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
            self._modules.append(DESCrackerModule(ip))

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
                     mask2: bytes | None = None):
        """
        Launches the exhaust on all workers.
        :param plaintext: plaintext to use
        :param ref1: first reference to check
        :param mask1: first mask to apply
        :param start: first key to exhaust (all 0x00 if None)
        :param end: last key to exhaust (all 0xFF if None)
        :param ref2: second reference to check
        :param mask2: second mask to apply
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

        # optional params handling
        start = start or bytes(7)
        end = end or bytes([0xFF] * 7)
        ref2 = ref2 or bytes([0xFF] * 8)
        mask2 = mask2 or bytes(8)

        # compute total number of workers
        total_workers = sum([m.workers_nbr for m in self._modules])

        self.logger.info(f"Launching exhaust from 0x{start.hex(' ', 4)} to 0x{end.hex(' ', 4)}"
                         f" on {total_workers} workers")

        # compute ranges of keys
        total_range = int.from_bytes(end) - int.from_bytes(start)
        each_range = total_range // total_workers
        ranges = [(int.from_bytes(start) + each_range * i, int.from_bytes(start) + each_range * (i + 1))
                  for i in range(total_workers)]

        # reset modules to clean them then set plaintext, refs and masks
        for m in self._modules:
            m.cmd_des_disable()
            m.cmd_des_set_plaintext(plaintext)
            assert m.cmd_des_get_plaintext() == plaintext
            m.cmd_des_set_ref(ref1, 1)
            m.cmd_des_set_ref(ref2, 2)
            m.cmd_des_set_mask(mask1, 1)
            m.cmd_des_set_mask(mask2, 2)

        # set keys for each worker
        module_idx = 0
        worker_idx = 0
        for sk, ek in ranges:
            m = self._modules[module_idx]
            m.cmd_des_set_start_key(sk.to_bytes(7), worker_idx)
            m.cmd_des_set_end_key(ek.to_bytes(7), worker_idx)
            if worker_idx >= m.workers_nbr:
                module_idx += 1
                worker_idx = 0
            else:
                worker_idx += 1

        # launch all workers
        for m in self._modules:
            m.cmd_des_reset()
            m.cmd_des_enable()

    def exhaust_keys_single_worker(self,
                                   module: DESCrackerModule,
                                   worker: int,
                                   plaintext: bytes,
                                   ref1: bytes,
                                   mask1: bytes,
                                   start: bytes | None = None,
                                   end: bytes | None = None,
                                   ref2: bytes | None = None,
                                   mask2: bytes | None = None):
        """
        Launches the exhaust on a single worker.
        :param module: module to work with
        :param worker: worker number to use
        :param plaintext: plaintext to use
        :param ref1: first reference to check
        :param mask1: first mask to apply
        :param start: first key to exhaust (all 0x00 if None)
        :param end: last key to exhaust (all 0xFF if None)
        :param ref2: second reference to check
        :param mask2: second mask to apply
        """
        # param checks
        if module not in self._modules:
            raise ValueError(f"Given module is not valid")
        if worker not in range(module.workers_nbr):
            raise ValueError(f"Worker number is not valid")
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

        # optional params handling
        start = start or bytes(7)
        end = end or bytes([0xFF] * 7)
        ref2 = ref2 or bytes([0xFF] * 8)
        mask2 = mask2 or bytes(8)

        self.logger.info(f"Launching exhaust from 0x{start.hex('_', 4)} to 0x{end.hex('_', 4)} on 1 worker")

        # reset module to clean it then set plaintext, refs and masks
        module.cmd_des_disable()
        module.cmd_des_set_plaintext(plaintext)
        module.cmd_des_set_ref(ref1, 1)
        module.cmd_des_set_ref(ref2, 2)
        module.cmd_des_set_mask(mask1, 1)
        module.cmd_des_set_mask(mask2, 2)

        # set keys
        module.cmd_des_set_start_key(start, worker)
        module.cmd_des_set_end_key(end, worker)

        # launch
        module.cmd_des_reset()
        module.cmd_des_enable()


    def check_exhaust(self) -> tuple[int, list[Result]]:
        """
        Checks if exhaust is running and returns results if any
        :return: DESCracker.STATUS_[NOT_]RUNNING, list of results
        """
        results = []
        running = False

        for m in self._modules:
            res_avail_all = m.cmd_des_result_available_all()
            ended_all = m.cmd_des_ended_all()

            for w in range(m.workers_nbr):
                res = (res_avail_all >> w) & 1
                ended = (ended_all >> w) & 1

                if res:
                    results.append(Result(*m.cmd_des_get_result(w)))

                if not ended:
                    running = True

        status = self.STATUS_RUNNING if running else self.STATUS_NOT_RUNNING

        self.logger.debug(f"Checked exhaust: status={'RUNNING' if status == self.STATUS_RUNNING else 'NOT_RUNNING'}")
        self.logger.debug(f"Results: {results}")

        return status, results


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
        self.exhaust_keys(plaintext, ref1, mask1, bytes(7), last_key)

        # wait until the end
        status = self.STATUS_RUNNING
        last_time_1s = time()
        while status == self.STATUS_RUNNING:
            status, _ = self.check_exhaust()
            if time() - last_time_1s > 1:
                last_time_1s = time()

                # temperatures
                for m in self._modules:
                    self.logger.info(f"Module{m.ip}: {m.cmd_get_temperature()}°C")

                # current keys
                for m in self._modules:
                    for w in range(m.workers_nbr):
                        self.logger.info(
                            f"Module{m.ip}, worker {w}: current key = {m.cmd_des_get_current_key(w).hex()}")

        # it's done!
        elapsed = time() - begin_time
        self.logger.info(f"Benchmark {'(all_match)' if all_match else ''} on {number_of_keys:_} finished in {elapsed:.02f}s!")
        self.logger.info(f"It corresponds to {int(number_of_keys / elapsed):_} keys/s")
        self.logger.info(f"Exhaust on 2**56 would take {pretty_str_seconds(int(2 ** 56 * elapsed / number_of_keys))}")
        return elapsed

    def test(self):
        """
        Tests if hardware works well. Raises an Exception if not.
        """
        self.logger.info(f"Beginning of tests")

        for m in self._modules:
            self.logger.info(f"Beginning of tests on module {m.ip}")

            for w in range(m.workers_nbr):
                # test if it stops when valid key is found on REF1 and REF2 then resume and end
                for ref_nbr in (1, 2):
                    self.logger.info(f"Testing single match on REF{ref_nbr}")
                    plaintext = bytes.fromhex('123456ABCD132536')
                    valid_key = bytes.fromhex('ab7420c266f36e')
                    mask = bytes([0xff] * 8)
                    ref = bytes.fromhex('C0B7A8D05F3A829C')
                    start_key = bytes.fromhex('ab7420c266f350')
                    end_key = bytes.fromhex('ab7420c266f380')

                    if ref_nbr == 1:
                        self.exhaust_keys_single_worker(m, w, plaintext, ref, mask, start_key, end_key)
                    else:
                        self.exhaust_keys_single_worker(m, w, plaintext,
                                                        bytes([0xff] * 8), bytes([0x00] * 8),
                                                        start_key, end_key,
                                                        ref, mask)

                    # test if it stops
                    begin_time = time()
                    while time() - begin_time < 1:
                        if m.cmd_des_result_available(w):
                            break
                        elif m.cmd_des_ended(w):
                            m.cmd_des_get_current_key(w)
                            raise DESCrackerError(f"Module{m.ip} (worker {w}) did not get a result")
                    else:
                        m.cmd_des_get_current_key(w)
                        raise DESCrackerError(f"Module{m.ip} (worker {w}) did not get a result")

                    res = m.cmd_des_get_result(w)
                    if res[0] != ref_nbr:
                        raise DESCrackerError(f"Expected result on REF{ref_nbr}, got REF{res[0]+1}")
                    if res[1] != valid_key:
                        raise DESCrackerError(f"Expected result {valid_key.hex()}, got {res[1].hex()}")

                    # test if it resumes when result is read
                    begin_time = time()
                    while time() - begin_time < 1:
                        if m.cmd_des_ended(w):
                            break
                    else:
                        m.cmd_des_get_current_key(w)
                        raise DESCrackerError(f"Module{m.ip} (worker {w}) did not resume and ended")

                # test 10 consecutive matches
                self.logger.info(f"Testing consecutive matches")
                plaintext = bytes.fromhex('123456ABCD132536')
                mask = bytes([0x00] * 8)
                ref = bytes([0x00] * 8)
                start_key = (0).to_bytes(7)
                end_key = (9).to_bytes(7)
                self.exhaust_keys_single_worker(m, w, plaintext, ref, mask, start_key, end_key)

                results = []
                begin_time = time()
                while time() - begin_time < 1:
                    if m.cmd_des_result_available(w):
                        results.append(m.cmd_des_get_result(w))
                    if len(results) == 10:
                        break
                else:
                    m.cmd_des_get_current_key(w)
                    raise DESCrackerError(f"Module{m.ip} (worker {w}) did not got 10 results within 1s (got {len(results)})")

                for i in range(10):
                    if results[i][1] != i.to_bytes(7):
                        raise DESCrackerError(f"Bad result found for key {i} "
                                              f"(expected {i.to_bytes(7).hex(' ', 4)}, got {results[i][1].hex(' ', 4)})")

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
    number = 10_000_000_000
    t = cracker.benchmark(number)
    print(f"Benchmark on {number:_} done in {t} seconds")

    number = 10_000
    t = cracker.benchmark(number, True)
    print(f"Benchmark (all match) on {number:_} done in {t} seconds")
