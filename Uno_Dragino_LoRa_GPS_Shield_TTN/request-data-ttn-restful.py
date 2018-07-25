#!/usr/bin/python

"""Send GPS and Sensor Data as valid LoRaWAN packet to TTN.

Change DEVICE, URL and ACCESS_KEYS['default key'] to the ones given
for your account and device. Currently it is defined for
@see https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/devices/test_01

Quick Start HowTo:
  - check coverage for Winterthur: https://www.thethingsnetwork.org/community/winterthur/
  - create an account: https://www.thethingsnetwork.org/docs/devices/uno/quick-start.html
  - create and register devices: http://wiki.dragino.com/index.php?title=Connect_to_TTN#Create_LoRa_end_devices
  - enable Data Storage (for 7 day): https://www.thethingsnetwork.org/docs/applications/storage/
  - use RESTful API: https://stackoverflow.com/questions/43693113/extracting-data-from-ttn-thethings-network-using-rest-api?rq=1
  - if needed reset frame counters in: https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/devices/test_01

$ python request-data-ttn-restful.py | tee ttn-test_01.log
"""

# https://console.thethingsnetwork.org/applications/vitudurum-drtrigon
DEVICE = 'test_01'
URL    = 'https://vitudurum-drtrigon.data.thethingsnetwork.org'
ACCESS_KEYS = {
    'default key': 'ttn-account-v2.cEu7sTqE1zCp2vKNcKiJY5salOzXk62GaSbnjSl1KBs',
}


import requests
import base64
import struct


# https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/integrations
# https://vitudurum-drtrigon.data.thethingsnetwork.org/
def requestRESTAPI(query):
    headers = {
        #'Accept': 'text/plain',
        'Accept': 'application/json',
        'Authorization': 'key ' + ACCESS_KEYS['default key'],
    }
    # ssl issue: https://stackoverflow.com/questions/10667960/python-requests-throwing-sslerror
    response = requests.get(URL + query, headers=headers, verify=False)
    data = None
    try:
        data = response.json()
    except ValueError:
        pass
    if not data:
        data = response.text
    return (response.status_code, data)


# https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/devices
# https://console.thethingsnetwork.org/applications/vitudurum-drtrigon/data

# Query the devices for which data has been stored
(status_code, data) = requestRESTAPI('/api/v2/devices')
print status_code
print data

## Query data
#(status_code, data) = requestRESTAPI('/api/v2/query')
#print status_code
#print data

# Query data for a specific device
#(status_code, data) = requestRESTAPI('/api/v2/query/%s' % DEVICE)          # default data of last 1h
(status_code, data) = requestRESTAPI('/api/v2/query/%s?last=7d' % DEVICE)  # all data as 7d are stored only
print status_code
if (status_code == 204):
    print "WARNING: no content (within given duration)"
#print data
for data_point in data:
    timestamp  = data_point['time']
    payload    = base64.b64decode(data_point['raw'])
    if len(payload) == 20:
        # for structs (e.g. containing 5 floats)
        #print map(ord, payload)
        print timestamp, ":", struct.unpack("fffff", payload)
    else:
        # for simple strings like "Hello World!"
        print timestamp, ": >", payload, "<", len(payload)
