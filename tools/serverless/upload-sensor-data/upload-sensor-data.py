import boto3
import os, sys

parent_dir = os.path.abspath(os.path.join(os.getcwd(), os.pardir))
sys.path.append(parent_dir)
home = os.path.expanduser("~")

from credentials import ACCESS_KEY, SECRET_KEY, PROJECT_NAME

from datetime import datetime
now = datetime.now()

session = boto3.Session(
  aws_access_key_id=ACCESS_KEY, 
  aws_secret_access_key=SECRET_KEY,
)

bucket_name = "{0}-data-bucket".format(PROJECT_NAME)

s3 = session.client('s3')

try:
  # NOTE: This must be the same path as given in config.SENSOR_VALUES_CSV_PATH in myno_web_app
  s3.upload_file(home + '/sensor_measures.csv', bucket_name, 'sensor_measures-{0}.csv'.format(now))
  os.remove(home + '/sensor_measures.csv')
  print("file uploaded")
except:
  print("error uploading file")