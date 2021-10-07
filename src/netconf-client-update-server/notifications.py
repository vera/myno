import datetime

__error_notifications = []
__info_notifications = []
__event_notifications = []

FADEOUT_TIME = 15 # seconds
TIMESTAMP_FORMAT = '%d.%m.%Y %H:%M:%S'

def add_error(error):
	global __error_notifications
	timestamp = datetime.datetime.now()
	__error_notifications.append((timestamp, error))

def get_errors():
	global __error_notifications
	__error_notifications = list(filter(lambda x: (datetime.datetime.now() - x[0]).seconds < FADEOUT_TIME, __error_notifications))
	return list(map(lambda x: (x[0].strftime(TIMESTAMP_FORMAT), x[1]), __error_notifications))

def add_info(info):
	global __info_notifications
	timestamp = datetime.datetime.now()
	__info_notifications.append((timestamp, info))

def get_info():
	global __info_notifications
	__info_notifications = list(filter(lambda x: (datetime.datetime.now() - x[0]).seconds < FADEOUT_TIME, __info_notifications))
	return list(map(lambda x: (x[0].strftime(TIMESTAMP_FORMAT), x[1]), __info_notifications))

def add_event(info):
	global __event_notifications
	timestamp = datetime.datetime.now()
	__event_notifications.append((timestamp, info))

def get_events():
	global __event_notifications
	__event_notifications = list(filter(lambda x: (datetime.datetime.now() - x[0]).seconds < FADEOUT_TIME, __event_notifications))
	return list(map(lambda x: (x[0].strftime(TIMESTAMP_FORMAT), x[1]), __event_notifications))