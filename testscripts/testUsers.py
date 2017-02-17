#!/usr/local/bin/python

import unittest
import httplib

g_base_url = "localhost:9980"


class HttpRequest:
    def __init__(self, api_name, verify_data):
        self.api_name = api_name
        self.__verify_result = verify_data
        self.http = httplib.HTTPConnection(g_base_url)
        self.status = 0
        self.response_msg = ""

    def send_request_POST(self, data, headers=None):
        import json

        if not headers:
            headers = {}

        data_str = json.dumps(data)

        self.http.request("POST", self.api_name, data_str, headers)
        response = self.http.getresponse()
        self.status = response.status
        self.response_msg = response.read()

        self.token = self.__verify_result(self.status, self.response_msg)
        if self.token is None:
            return False
        else:
            return True

    def send_request_GET(self, headers=None):
        if not headers:
            headers = {}

        self.http.request("GET", self.api_name, None, headers)
        response = self.http.getresponse()
        self.status = response.status
        self.response_msg = response.read()
        return self.response_msg

    def response(self):
        return "[Status: %d, Response: %s]" % (self.status, self.response_msg)


class TestUserRegister(unittest.TestCase):

    def setUp(self):
        self.http_request = HttpRequest("/api/user/register", self.__verify_result)

    def __verify_result(self, status, response):
        import json

        if status != 200:
            return None

        data = json.loads(response)
        jwt = data.get("jwt", None)
        if jwt is None:
            return None

        return jwt

    def test_register_invalid_user(self):
        json_data = {
            "email": "test1",
            "password": "123456"
        }
        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_register_invalid_input1(self):
        json_data = {
            "email1": "test1",
            "password": "123456"
        }

        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_register_invalid_input2(self):
        json_data = {
            "email": "test1",
            "password1": "123456"
        }

        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_register_user(self):
        json_data = {
            "email": "test109191@gmail.com",
            "password": "123456"
        }
        self.assertTrue(self.http_request.send_request_POST(json_data), self.http_request.response())

        # remove user
        token = self.http_request.token
        request = HttpRequest("/api/user/remove", self.__verify_result)
        headers = {
            "Authorization": "Bearer " + token
        }
        request.send_request_POST({}, headers)


class TestUserLogin(unittest.TestCase):

    def setUp(self):
        self.http_request = HttpRequest("/api/user/login", self.__verify_result)

        # create new user
        request = HttpRequest("/api/user/register", self.__verify_result)
        json_data = {
            "email": "test1234@gmail.com",
            "password": "123456"
        }

        request.send_request_POST(json_data)
        self.user_token = request.token
        # print("Login token: %s" % self.user_token)

    def tearDown(self):
        # remove user
        request = HttpRequest("/api/user/remove", self.__verify_result)
        json_data = {
        }
        headers = {
            "Authorization": "Bearer " + self.user_token
        }
        request.send_request_POST(json_data, headers)

    def __verify_result(self, status, response):
        import json

        if status != 200:
            return None

        # print(response)
        data = json.loads(response)
        jwt = data.get("jwt", None)
        if jwt is None:
            return None

        return jwt

    def test_login_invalid_user(self):
        json_data = {
            "email": "test1",
            "password": "123456"
        }
        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_login_invalid_input1(self):
        json_data = {
            "email1": "test1",
            "password": "123456"
        }

        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_login_invalid_input2(self):
        json_data = {
            "email": "test1",
            "password1": "123456"
        }

        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_login_user(self):
        json_data = {
            "email": "test1234@gmail.com",
            "password": "123456"
        }
        self.assertTrue(self.http_request.send_request_POST(json_data), self.http_request.response())

    def test_login_non_existing_user(self):
        json_data = {
            "email": "test123411111@gmail.com",
            "password": "123456"
        }
        self.assertFalse(self.http_request.send_request_POST(json_data), self.http_request.response())


class TestUserWidget(unittest.TestCase):

    def setUp(self):
        # create new user
        request = HttpRequest("/api/user/login", self.__verify_login_result)
        json_data = {
            "email": "huuhoa@gmail.com",
            "password": "112233"
        }

        request.send_request_POST(json_data)
        self.user_token = request.token
        self.headers = {
            "Authorization": "Bearer " + self.user_token
        }
        # print("Login token: %s" % self.user_token)

    def tearDown(self):
        # remove user
        request = HttpRequest("/api/user/logout", self.__verify_widget_result)
        json_data = {
        }
        headers = {
            "Authorization": "Bearer " + self.user_token
        }
        request.send_request_POST(json_data, headers)
        pass

    def __verify_login_result(self, status, response):
        import json

        if status != 200:
            return None

        # print(response)
        data = json.loads(response)
        jwt = data.get("jwt", None)
        if jwt is None:
            return None

        return jwt

    def __verify_widget_result(self, status, response):
        if status != 200:
            return None

        print(response)

        return response

    def test_add_widget(self):
        json_data = {
            "action": "widgets.add",
            "type": 1,
            "device_id": 12,
            "variable_id": 10
        }
        http_request = HttpRequest("/api/web/widget", self.__verify_widget_result)
        self.assertTrue(http_request.send_request_POST(json_data, self.headers), http_request.response())

    def test_get_widget(self):
        import json

        http_request = HttpRequest("/api/web/widget/2", self.__verify_widget_result)
        response = http_request.send_request_GET(self.headers)
        self.assertEqual(http_request.status, 200, http_request.response())

        obj = json.loads(response)
        self.assertEqual(obj['data']['device_id'], 12)
        self.assertEqual(obj['data']['variable_id'], 10)


    def test_get_all_widgets(self):
        import json

        http_request = HttpRequest("/api/web/widget", self.__verify_widget_result)
        response = http_request.send_request_GET(self.headers)
        self.assertEqual(http_request.status, 200, http_request.response())

        print(response)


def __my_verify_result(status, response):
    import json

    if status != 200:
        return None

    # print(response)
    data = json.loads(response)
    jwt = data.get("jwt", None)
    if jwt is None:
        return None

    return jwt


def __my_verify_device_result(status, response):
    import json

    print(status, response)

    return response

user_token = ""


def login():
    global user_token
    request = HttpRequest("/api/user/login", __my_verify_result)
    json_data = {
        "email": "huuhoa@gmail.com",
        "password": "112233"
    }

    request.send_request_POST(json_data)
    user_token = request.token
    print("Login token: %s" % user_token)


def toggleDeviceState(device_id, var_id, value):
    global user_token
    headers = {
        "Authorization": "Bearer " + user_token
    }

    request = HttpRequest("/api/http", __my_verify_device_result)
    json_data = {
        "action":"update",
        "deviceid": device_id,
        "apikey": "4b1cec12-d54b-11e5-b276-80e650230236",
        "params": {
            "varId": var_id,
            "value": value
        }
    }

    print("%r " % request.send_request_POST(json_data, headers))


# def print_test_result(f, expected=True):
#     from termcolor import colored
#     if f() == expected:
#         print(colored("%s: OK" % (f.__name__), "green"))
#     else:
#         print(colored("%s: FAILED" % (f.__name__), "red"))

if __name__ == "__main__":
    unittest.main()

