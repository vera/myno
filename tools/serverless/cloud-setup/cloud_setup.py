import json, boto3, os, sys

parent_dir = os.path.abspath(os.path.join(os.getcwd(), os.pardir))
sys.path.append(parent_dir)
home = os.path.expanduser("~")

from credentials import ACCESS_KEY, SECRET_KEY, SENDER_EMAIL_ADDRESS, RECEIVER_EMAIL_ADDRESSES, PROJECT_NAME, REGION
from zipfile import ZipFile

session = boto3.Session(
    aws_access_key_id=ACCESS_KEY,
    aws_secret_access_key=SECRET_KEY,
)

# prepare emails to save as env variables
receiver_email_as_string = ";".join(RECEIVER_EMAIL_ADDRESSES)
lambda_env = {
        'SENDER_EMAIL_ADDRESS': SENDER_EMAIL_ADDRESS,
        'RECEIVER_EMAIL_ADDRESSES': receiver_email_as_string
      }

user = session.client('sts').get_caller_identity().get('Account')

# Initiate IAM client to create role and policy to attach to Lambda function
iam = session.client('iam')
role_name = "{0}-role".format(PROJECT_NAME)
policy_name = "{0}_policy".format(PROJECT_NAME)
# Check if role already exists. If no create new one
try:
  role = iam.get_role(RoleName = role_name)["Role"]
except:
  assume_role_policy_document = json.dumps({
      "Version": "2012-10-17",
      "Statement": [
          {
          "Effect": "Allow",
          "Principal": {
              "Service": "lambda.amazonaws.com"
          },
          "Action": "sts:AssumeRole"
          }
      ]
  })

  role = iam.create_role(
      RoleName = role_name,
      AssumeRolePolicyDocument = assume_role_policy_document
  )["Role"]["RoleName"]
  print("role created")

#check if policy is attached to role. If not create it 
attached_policies = iam.list_attached_role_policies(RoleName = role_name)["AttachedPolicies"]
if not any(policy['PolicyName'] == policy_name for policy in attached_policies):
  try:
    policy_document = json.dumps({
      "Version": "2012-10-17",
      "Statement": [
          {
              "Effect": "Allow",
              "Action": [
                  "logs:*"
              ],
              "Resource": "arn:aws:logs:*:*:*"
          },
          {
              "Effect": "Allow",
              "Action": [
                  "s3:*"
              ],
              "Resource": "arn:aws:s3:::*"
          },
          {
              "Effect": "Allow",
              "Action": [
                  "lambda:*"
              ],
              "Resource": "*"
          },
          {
            "Effect": "Allow",
            "Action": [
                "ses:*"
            ],
            "Resource": "*"
        }
      ]
    })
      
    policy = iam.create_policy(
      PolicyName = policy_name,
      PolicyDocument = policy_document
    )["Policy"]

    attach_role_policy_response = iam.attach_role_policy(
        RoleName = role["RoleName"],
        PolicyArn = policy["Arn"]
    )
    print("policy created and attached to role")
  except:
    print("error creating policy")

# initiate s3 bucket to store function code
s3 = session.client('s3')
function_bucket_name = "{0}-function-code-bucket".format(PROJECT_NAME)
data_bucket_name = "{0}-data-bucket".format(PROJECT_NAME)

if not any(b["Name"] == function_bucket_name for b in s3.list_buckets()["Buckets"]):
  code_bucket = s3.create_bucket(
    Bucket = function_bucket_name,
    CreateBucketConfiguration={
        'LocationConstraint': REGION,
    },
  )
  print("Code Bucket created")

# initiate s3 bucket to store data
if not any(b["Name"] == data_bucket_name for b in s3.list_buckets()["Buckets"]):
  data_bucket = s3.create_bucket(
    Bucket = data_bucket_name,
    CreateBucketConfiguration={
        'LocationConstraint': REGION,
    },
  )
  print("Data Bucket created")

aws_lambda = session.client('lambda')
runtime = 'python3.8'
layer_name = "{0}-layer".format(PROJECT_NAME)

#create lambda layer that contains panda package
try:
  layer_arn = aws_lambda.get_layer_version(LayerName=layer_name, Version=1)["LayerArn"]
except:
  s3.upload_file('python.zip', function_bucket_name, 'python.zip')
  layer_arn = aws_lambda.publish_layer_version(
    LayerName = layer_name,
    Content={
      'S3Bucket': function_bucket_name,
      'S3Key': 'python.zip',
    },
    CompatibleRuntimes=[runtime]
  )['LayerVersionArn']

# create lambda function to send email for occuring event
event_lambda_function_name = "lambda_function_event"
event_lamda_name = "{0}-event-lambda".format(PROJECT_NAME)

try:
  event_lambda_function = aws_lambda.get_function(FunctionName=event_lamda_name)["Configuration"]
except:
  zip_name = "{0}.zip".format(event_lambda_function_name)
  zip_obj = ZipFile(zip_name, 'w')
  for folder, subfolder, filenames in os.walk(event_lambda_function_name):
    for filename in filenames:   
      zip_obj.write("{0}/{1}".format(event_lambda_function_name, filename), "{0}".format(filename))
  
  zip_obj.close()

  s3.upload_file(zip_name, function_bucket_name, zip_name)
  os.remove(zip_name)

  event_lambda_function = aws_lambda.create_function(
    FunctionName=event_lamda_name,
    Runtime=runtime,
    MemorySize=256,
    Role=role["Arn"],
    Code={
      'S3Bucket': function_bucket_name,
      'S3Key': zip_name,
    },
    Handler="{0}.handler".format(event_lambda_function_name),
    Environment={
      'Variables': lambda_env
    },
    Layers=[layer_arn]
  )

  print("Event Lambda Function created")

summary_lamda_name = "{0}-summary-lambda".format(PROJECT_NAME)
summary_lambda_function_name = "lambda_function_summary"

try:
  summary_lambda_function = aws_lambda.get_function(FunctionName=summary_lamda_name)["Configuration"]
except:
  summary_zip_name = "{0}.zip".format(summary_lambda_function_name)
  zip_obj = ZipFile(summary_zip_name, 'w')
  for folder, subfolder, filenames in os.walk(summary_lambda_function_name):
    for filename in filenames:   
      zip_obj.write("{0}/{1}".format(summary_lambda_function_name, filename), "{0}".format(filename))
  
  zip_obj.close()

  s3.upload_file(summary_zip_name, function_bucket_name, summary_zip_name)
  os.remove(summary_zip_name)

  lambda_env["BUCKET_NAME"] = data_bucket_name

  summary_lambda_function = aws_lambda.create_function(
    FunctionName=summary_lamda_name,
    Runtime=runtime,
    MemorySize=256,
    Role=role["Arn"],
    Code={
      'S3Bucket': function_bucket_name,
      'S3Key': summary_zip_name,
    },
    Handler="{0}.handler".format(summary_lambda_function_name),
    Environment={
      'Variables': lambda_env
    },
    Layers=[layer_arn]
  )

  print("Summary Lambda Function created")

events = session.client('events')
trigger_name = "{0}-trigger".format(PROJECT_NAME)

rules = events.list_rules(NamePrefix = trigger_name)["Rules"]

if (len(rules) == 0):
  trigger_arn = events.put_rule(
    Name = trigger_name,
    ScheduleExpression = "cron(0 10 ? * FRI *)",
    State = 'ENABLED',
  )["RuleArn"]
  print("rule created")
else:
  trigger_arn = rules[0]["Arn"]

try:
  aws_lambda.add_permission(
    FunctionName = summary_lamda_name,
    StatementId = "{0}-event".format(trigger_name),
    Action = 'lambda:InvokeFunction',
    Principal = 'events.amazonaws.com',
    SourceArn = trigger_arn,
  )

  events.put_targets(
      Rule=trigger_name,
      Targets=[
          {
              'Id': "1",
              'Arn': summary_lambda_function["FunctionArn"],
          },
      ]
  )
except Exception as ex:
    if type(ex).__name__ == "ResourceConflictException":
      print("Permission already added")
    else:
      print("error adding permission")

try:
  # Creating API
  api_gateway = session.client('apigatewayv2')

  api_name = "{0}-event-api".format(PROJECT_NAME)

  event_api = api_gateway.create_api(
    Name=api_name,
    ProtocolType="HTTP",
    RouteKey="POST /event",
    Target=event_lambda_function["FunctionArn"]
  )

  try:
    aws_lambda.add_permission(
      FunctionName = event_lamda_name,
      StatementId = api_name,
      Action = 'lambda:InvokeFunction',
      Principal = 'apigateway.amazonaws.com',
      SourceArn = "arn:aws:execute-api:{0}:{1}:{2}/*/*/event".format(REGION, user, event_api["ApiId"]),
    )
  except Exception as ex:
    if type(ex).__name__ == "ResourceConflictException":
      print("Permission for event already added")
    else:
      print("error adding permission")
      print(ex)

  print("API for events created! Use the following address: " + event_api["ApiEndpoint"] + "/event")

except Exception as ex:
  print("error creating API")
  print(ex)

ses = session.client('ses')

verified_addresses = ses.list_verified_email_addresses()["VerifiedEmailAddresses"]

if SENDER_EMAIL_ADDRESS not in verified_addresses:
  ses.verify_email_address(EmailAddress = SENDER_EMAIL_ADDRESS)

for address in RECEIVER_EMAIL_ADDRESSES:
  if address not in verified_addresses:
    ses.verify_email_address(EmailAddress = address)
