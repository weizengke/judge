
import requests
import json
import sys, re, os, time

user = "xxxxxxxx" 
password = "xxxxxxxx"

#题目字典
questionDict = {"3":"bitwise-and-of-numbers-range", }

def login(user, password): 
    client = requests.session()
    client.encoding = "utf-8"
    sign_in_url = 'https://leetcode-cn.com/accounts/login/'
    client.get(sign_in_url, verify=False)

    login_data = {
        'login': user, 
        'password': password
    }
    
    result = client.post(sign_in_url, data=login_data, headers=dict(Referer=sign_in_url))
    if result.ok:
        print ("Login successfully.")
    return client

def GetResult(submitId, requests_session):
    headers = {
        "Content-Type": "application/json"
    }

    url = "https://leetcode-cn.com/submissions/detail/"+str(submitId)+"/check/"
    print (url)

    h = requests_session.get(url = url, headers = headers, verify=False).json()
    return h

def getStatus(submitId, client, resultJson):
    cnt = 0
    result = ""
    while True:
        time.sleep(5)
        cnt = cnt + 1;
        if cnt > 12:
            break;

        result = GetResult(submitId, client)
        #print(result)
        #print(result.get("state"))
        
        if result.get("status_msg") == "Compile Error":
            #print (result.get("full_compile_error"))
            break;

        if result.get("state") == "SUCCESS":
            #print ("memory=", result.get("memory"), ", time=", result.get("status_runtime"))
            break;

    with open(resultJson, 'w') as file_object:
        file_object.write(str(json.dumps(result)))

def PostCode(answner, questionId, question, client):
    print("post code:", questionId, " - ", question)
    url = "https://leetcode-cn.com/problems/"+question+"/submit/"
    referer = "https://leetcode-cn.com/problems/"+question+"/submissions/"
    headers = {
        "Content-Type": "application/json",
        "referer": referer
    }
    body = {
        "question_id": questionId,
        "lang": "c",
        "typed_code": answner,
        "test_mode": "false",
        "test_judger": "",
        "questionSlug": question
    }
    ret = client.post(url = url, headers = headers, data=json.dumps(body), verify=False)
    #print(ret)
    #print(ret.text)
    return json.loads(ret.text).get("submission_id")

def getCode(path):
    with open(path) as file_obj:
        content = file_obj.read()
        #print(content)
        return content

if __name__ == "__main__":
    requests.packages.urllib3.disable_warnings()

    user = sys.argv[1]
    password = sys.argv[2]
    questionId = sys.argv[3]
    language = sys.argv[4]
    solutionPath = sys.argv[5]
    resultPath = sys.argv[6]

    client = login(user,password)
    submitId = PostCode(getCode(solutionPath), questionId, questionDict[questionId], client)
    if not submitId is None:
        getStatus(submitId, client, resultPath)

    