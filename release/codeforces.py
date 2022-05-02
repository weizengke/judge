
import requests
import json
import sys, re, os, time
import datetime
from bs4 import BeautifulSoup

from selenium import webdriver
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support.select import Select

import logging
logger = logging.getLogger(__name__)
logger.setLevel(level = logging.INFO)
handler = logging.FileHandler("codeforces_log.txt")
handler.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
logger.addHandler(handler)

option = webdriver.ChromeOptions()
#option.add_argument('--disable-gpu')
option.add_argument('--headless')
option.add_argument('--ignore-certificate-errors')
option.add_argument('-ignore -ssl-errors')
option.add_experimental_option('excludeSwitches', ['enable-logging'])
#option.add_experimental_option('useAutomationExtension',False) 
option.add_argument('log-level=3')
browser = webdriver.Chrome(chrome_options=option)
#browser = webdriver.Chrome('c:\chromedriver.exe')

def login(user, password, cnt): 
    try:
        logger.info("Start login user %s, password %s, cnt %d.", user, password, cnt)
        wait = WebDriverWait(browser, 20)
        browser.get("https://codeforces.com/enter")
        wait.until(lambda browser: browser.find_element_by_name("handleOrEmail")).send_keys(user)
        passwd = browser.find_element_by_name("password")
        passwd.send_keys(password)
        login_button = browser.find_element_by_xpath("//*[@id='enterForm']/table/tbody/tr[4]/td/div[1]/input")
        login_button.click()
        logger.info("Login %s ok.", user)
        time.sleep(2)
    except:
        if cnt > 0:
            logger.info("Login again.")
            login(user, password, cnt - 1)
        else:
            logger.error("Login timeout.")

def submit(contestId, problemIndex, languageId, source, cnt):
    try:
        logger.info("Start submit problem %s%s, programTypeId %s, cnt %d.", contestId, problemIndex, languageId, cnt)
        source = "// " + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') + "\n" +   source    
        wait = WebDriverWait(browser, 30)
        url = "https://codeforces.com/problemset/submit/" + contestId + "/" + problemIndex
        logger.info(url)
        browser.get(url)
        programTypeId = wait.until(lambda browser: browser.find_element_by_name("programTypeId"))
        Select(programTypeId).select_by_value(languageId)
        logger.info("Start input code")
        js = 'var ucode = document.getElementById("sourceCodeTextarea"); ucode.value=arguments[0]'
        browser.execute_script(js, source)
        logger.info("Start submit_button click")
        submit_button = browser.find_element_by_class_name("submit")
        submit_button.click()
        logger.info("Submit ok")
    except:
        if cnt > 0:
            logger.info("Submit again")
            submit(contestId, problemIndex, languageId, source, cnt-1)
        else:
            logger.error("Submit timeout.")

def getLastSubmissionId(user):
    submission = "0"
    try:
        submission = getLastSubmission(user)
        return submission[0]
    except:
        return "0"

def getLastSubmission(user):
    row_list = []
    try:
        logger.info("Start get %s last submission.", user)
        wait = WebDriverWait(browser, 60)
        browser.get("https://codeforces.com/submissions/" + user)

        menu_table = browser.find_element_by_xpath("//*[@id='pageContent']/div[4]/div[6]/table")
        rows = menu_table.find_elements_by_tag_name('tr')
        before_add_numbers = len(rows)
        for tr in rows:
            table_td_list = tr.find_elements_by_tag_name("td")
            row_numbers = len(table_td_list)
            if row_numbers > 0:
                for td in table_td_list:
                    row_list.append(td.text)
                break
        logger.info("End get %s last submission %s. ", user, row_list[0])
        return row_list
    except:
        logger.error("get last submission failed")
        return row_list
           
def saveLastSubmissionResult(user, contestId, resultJson, lastSubmissionId):
    result = []
    try:
        cnt = 0
        conpile_err = ""
        while True:
            time.sleep(5)
            cnt = cnt + 1;
            if cnt > 12:
                break;

            result = getLastSubmissionEx(user)
            if lastSubmissionId == result[0]:
                logger.error("lastSubmissionId is the same.")
                return

            logger.info(result)
            if result[5].find("Compilation error")>=0:
                conpile_err = getCompileError(contestId, result[0])
                break;

            if result[5].find("Wrong answer")>=0 or result[5].find("Runtime error")>=0:
                break;

            if result[5].find("Time limit exceeded")>=0 or result[5].find("Output limit exceeded")>=0 or result[5].find("Memory limit exceeded")>=0:
                break;

            if result[5].find("Accepted")>=0:
                break;

        log = getJudgeLog(contestId, result[0])

        status_dict = {'submissionid': '', 'submittime':'','username':'', 'problem': '', 'lang':'', 'verdict':'', 'time':'', 'memory':''}
        status_dict['submissionid'] = result[0]
        status_dict['submittime'] = result[1]
        status_dict['username'] = result[2]
        status_dict['problem'] = result[3]
        status_dict['lang'] = result[4]
        status_dict['verdict'] = result[5]
        status_dict['time'] = result[6]
        status_dict['memory'] = result[7]
        status_dict['compile_error'] = conpile_err
        status_dict['testcases'] = log
        with open(resultJson, 'w') as file_object:
            file_object.write(str(json.dumps(status_dict)))
    except:
        logger.error("get last submission status failed")

def getCode(path):
    with open(path) as file_obj:
        content = file_obj.read()
        return content

def getCompileError(contestId, submissionId):
    try:
        logger.info("Start get submission %s compile info.", submissionId)
        wait = WebDriverWait(browser, 60)
        url = "https://codeforces.com/contest/" + contestId + "/submission/" + submissionId
        browser.get(url)
        click = wait.until(lambda browser: browser.find_element_by_xpath("//*[@id='pageContent']/div[4]/div[2]/a"))
        click.click()
        time.sleep(2)
        checkerComment = browser.find_element_by_xpath("//*[@id='pageContent']/div[4]/div[3]/div[9]/div[2]/pre")
        #print("checkerComment", checkerComment.text)
        return checkerComment.text
    except:
        logger.error("Get submission %s compile info error.", submissionId)
        return ""

def getJudgeLog(contestId, submissionId):
    log = []
    try:
        logger.info("Start get submission %s judge log.", submissionId)
        wait = WebDriverWait(browser, 60)
        url = "https://codeforces.com/contest/" + contestId + "/submission/" + submissionId
        browser.get(url)
        click = wait.until(lambda browser: browser.find_element_by_xpath("//*[@id='pageContent']/div[4]/div[2]/a"))
        click.click()
        time.sleep(2)

        num = 3
        while True:
            testcase_xpath = "//*[@id='pageContent']/div[4]/div[" + str(num) + "]"
            testcase = browser.find_elements_by_xpath(testcase_xpath)
            if len(testcase) == 0:
                break;
            #print(testcase[0].text)
            log.append(testcase[0].text)
            num += 1
        logger.info("End get submission %s judge log. (caseNum=%d).", submissionId, len(log))
        return log            
    except:
        logger.error("Get submission %s judge log error.", submissionId)
        return log 

def codeforces_judge(user, password, problem, language, solutionPath, resultPath):
    try:
        logger.info("Codeforces virtual judge start. user:%s.", user)

        login(user,password, 5)
        lastSubmissionId = getLastSubmissionIdEx(user)
        if lastSubmissionId != "":
            contestId = re.sub("\D", "", problem) 
            problemIndex = ''.join(re.findall(r'[A-Za-z]', problem))
            submit(contestId, problemIndex, language, getCode(solutionPath), 5)
            saveLastSubmissionResult(user, contestId, resultPath, lastSubmissionId)

        logger.info("Codeforces virtual judge end. user:%s.", user)
        return 1
    except:
        return 0

def getLastSubmissionEx(user):
    logger.info("Start get user %s last submission.", user)
    client = requests.session()
    headers = {
        "Content-Type": "text/html;charset=UTF-8",
        "referer":"https://codeforces.com/profile/"+user
    }
    url = "https://codeforces.com/submissions/"+user

    h = client.get(url = url, headers = headers, verify=False)
    
    submission = []
    soup = BeautifulSoup(h.text,'html.parser')
    table = soup.find('table',{'class':'status-frame-datatable'})
    for el in soup.findAll('tr'):
        submissionId = el.find('td', {'class':'id-cell'})
        if submissionId is None:
            continue
        else:
            for it in el.findAll('td'):
                submission.append(it.text.strip())
            logger.info("End get user %s last submission %s.", user, submission)
            return submission
    logger.info("End get user %s last submission failed.", user)
    return submission

def getLastSubmissionIdEx(user):
    logger.info("Start get user %s last submission id.", user)
    submission = getLastSubmissionEx(user)
    if len(submission) == 0:
        logger.info("End get user %s last submission id failed.", user)
        return ""
    else:
        logger.info("End get user %s last submission id %s.", user, submission[0])
        return submission[0]

if __name__ == "__main__":
    requests.packages.urllib3.disable_warnings()

    user = sys.argv[1]
    password = sys.argv[2]
    problem = sys.argv[3]
    language = sys.argv[4]
    solutionPath = sys.argv[5]
    resultPath = sys.argv[6]

    ret = codeforces_judge(user, password, problem, language, solutionPath, resultPath)
    if ret == 0:
        codeforces_judge(user, password, problem, language, solutionPath, resultPath)

    browser.close()


    