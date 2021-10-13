# NETCONF client + Web UI for MYNO

## Prerequisites

To install the required packages:

```
pip3 install -r myno_web_app/requirements.txt
```

## Steps

To run the web server:

```
pip3 install -e .
FLASK_APP=myno_web_app flask run --host=0.0.0.0
```

## Testing

To run the tests:

```
pytest
```