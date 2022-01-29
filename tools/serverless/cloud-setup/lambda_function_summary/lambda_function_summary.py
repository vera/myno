import boto3, os, io
import pandas as pd

def handler(event, context):

  s3 = boto3.client("s3")

  BUCKET_NAME = os.environ.get('BUCKET_NAME')

  files = s3.list_objects_v2(Bucket=BUCKET_NAME)["Contents"]

  data = []
  for s3_file in files:
    file_content = s3.get_object(Bucket=BUCKET_NAME,Key=s3_file["Key"])["Body"].read()
    data.append(pd.read_csv(io.BytesIO(file_content), header=None, names=["timestamp", "topic", "message"]))
    s3.delete_object(Bucket=BUCKET_NAME,Key=s3_file["Key"])
  data = pd.concat(data)

  #split topic field
  data[['type', 'topic_name', 'topic_id', 'sensor_id']] = data.topic.str.split('/', expand=True)
  #convert values to numbers
  data['value'] = data['message'].astype(float)
  #drop all columns but value and topic_name
  data = data.drop(['topic', 'type', 'topic_id', 'sensor_id', 'timestamp', 'message'], axis=1)

  values = pd.DataFrame()
  values['mean'] = data.groupby('topic_name').median()['value']
  values['min'] = data.groupby('topic_name').min()['value']
  values['max'] = data.groupby('topic_name').max()['value']

  body = "Hier ist Ihre monatliche Zusammenfassung der Sensorwerte:\n"

  for i, row in values.iterrows():
    body += "{0}\nMedian: {1}, Minimum: {2}, Maximum: {3}\n".format(i, row['mean'], row['min'], row['max'])

  ses = boto3.client('ses')

  SENDER_EMAIL_ADDRESS = os.environ.get('SENDER_EMAIL_ADDRESS')
  RECEIVER_EMAIL_ADDRESSES = os.environ.get('RECEIVER_EMAIL_ADDRESSES')

  ses.send_email(
    Source = SENDER_EMAIL_ADDRESS,
    Destination = {'ToAddresses': RECEIVER_EMAIL_ADDRESSES.split(';')}, 
    Message = {
      'Subject': {'Data': 'Sensorwerte Zusammenfassung'},
      'Body': {'Text': {'Data': body}}
    }
  )