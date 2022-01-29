import boto3, os, json

def handler(event, context):

  body = "Ein neues Event ist aufgetreten:\n"
  body += json.loads(event["body"])["event"]

  ses = boto3.client('ses')

  SENDER_EMAIL_ADDRESS = os.environ.get('SENDER_EMAIL_ADDRESS')
  RECEIVER_EMAIL_ADDRESSES = os.environ.get('RECEIVER_EMAIL_ADDRESSES')

  ses.send_email(
    Source = SENDER_EMAIL_ADDRESS,
    Destination = {'ToAddresses': RECEIVER_EMAIL_ADDRESSES.split(';')}, 
    Message = {
      'Subject': {'Data': 'Neues Myno Event'},
      'Body': {'Text': {'Data': body}}
    }
  )