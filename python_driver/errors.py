class DESCrackerError(Exception):
    pass


class DESCrackerTimeoutError(DESCrackerError):
    def __init__(self, msg, results):
        DESCrackerError.__init__(self, msg)
        self.results = results


class HWVersionError(DESCrackerError):
    pass
