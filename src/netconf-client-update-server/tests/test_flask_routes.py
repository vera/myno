import pytest

from myno_web_app import create_app

@pytest.fixture
def client():
  app = create_app()
  yield app.test_client()

def test_root_is_200(client):
  res = client.get('/')
  assert res.status_code == 200

def test_root_is_empty(client):
  res = client.get('/')
  assert b'No devices available, press "Get Devices" to refresh.' in res.get_data()