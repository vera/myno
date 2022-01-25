# NETCONF client + Web UI for MYNO

## Prerequisites

To install the required packages and the web server application:

```
pip3 install -r myno_web_app/requirements.txt
pip3 install -e .
```

To install the binary used for signing manifests:

```
cd ../../tools/myno-update-protocol/micro-ecc-signature
make
cd -
ln -s ../../tools/myno-update-protocol/micro-ecc-signature/createSig .
```

## Steps

To run the web server:

```
FLASK_APP=myno_web_app flask run --host=0.0.0.0
```

Or simply:

```
./run.sh
```

## Testing

To run the tests:

```
pytest
```