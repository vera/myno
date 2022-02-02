from ecdsa import SigningKey, NIST256p
from hashlib import sha256

DEFAULT_KEY_ID_VENDOR = 0xCAFECAFE
DEFAULT_KEY_ID_UPDATE_SERVER = 0xDEADBEEF

sk_vendor = {
    0xCAFECAFE: SigningKey.from_string(bytes.fromhex("901a2e11e1b654fe150f4d76826673e1216c579a3c46a87637c76e7a6367ed5d"), curve=NIST256p, hashfunc=sha256),
    0xC007CAFE: SigningKey.from_string(bytes.fromhex("d89032fa54209d00356e0140529d9049d0b5a6a1a181b326d16be69263d0f11d"), curve=NIST256p, hashfunc=sha256)
}

sk_update_server = {
    0xDEADBEEF: SigningKey.from_string(bytes.fromhex("1b7bab8d66a177a9ce60740a1fecbf69f7a8701b50116f2d0d25ff82f6d063b0"), curve=NIST256p, hashfunc=sha256),
    0xFEEDBEEF: SigningKey.from_string(bytes.fromhex("33569de8139acc9d74920fb47cb2d18ff9c3db426faddf2b8234a5f8d3b572ae"), curve=NIST256p, hashfunc=sha256)
}