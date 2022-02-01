from ecdsa import SigningKey, NIST256p
from hashlib import sha256

sk_update_server = SigningKey.from_string(bytes.fromhex("1b7bab8d66a177a9ce60740a1fecbf69f7a8701b50116f2d0d25ff82f6d063b0"), curve=NIST256p, hashfunc=sha256)
sk_vendor = SigningKey.from_string(bytes.fromhex("901a2e11e1b654fe150f4d76826673e1216c579a3c46a87637c76e7a6367ed5d"), curve=NIST256p, hashfunc=sha256)