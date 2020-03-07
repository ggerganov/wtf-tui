/*! \file templates.h
 *  \brief Enter description here.
 */

#pragma once

#include <vector>
#include <string>

namespace Core {

struct Template {
    std::string name = "";
    std::string type = "";
    std::string desc = "";
    std::string config = "";
};

std::vector<Template> kTemplatesOriginal = {
    {
        "### Custom templates",
        "null",
        "",
        R"(
)",
    },
    {
        "Uptime",
        "cmdrunner",
        "Machine uptime",
        R"(
enabled: true
position:
  top: 21
  left: 0
  width: 87
  height: 20
args: []
cmd: uptime
refreshInterval: 30
)",
    },
    {
        "Disk usage",
        "cmdrunner",
        "Disk usage",
        R"(
enabled: true
position:
  top: 21
  left: 0
  width: 87
  height: 20
args: [-h]
cmd: df
refreshInterval: 300
)",
    },
    {
        "List folder",
        "cmdrunner",
        "List current folder contents",
        R"(
enabled: true
position:
  top: 21
  left: 0
  width: 87
  height: 20
args: [-l, ./]
cmd: ls
refreshInterval: 5
)",
    },
    {
        "Github Issues/PRs",
        "github",
        "Lists the assigned Issues and PRs to a user",
        R"(
enabled: true
position:
  top: 0
  left: 0
  width: 58
  height: 21
apiKey: <placeholder>
baseURL: ""
customQueries:
  openIssues:
    title: My Issues
    perPage: 8
    filter: is:issue state:open author:ggerganov
enableStatus: true
refreshInterval: 300
repositories:
  - ggerganov/wtf-tui
uploadURL: ""
username: ggerganov
)",
    },
    {
        "### Original templates",
        "null",
        "",
        R"(
)",
    },
    {
        "Azure DevOps",
        "azuredevops",
        "https://wtfutil.com/modules/azure_devops/",
        R"(
apiToken: "<placeholder>"
enabled: true
labelColor: "lightblue"
maxRows: 3
orgUrl: "https://dev.azure.com/myawesomecompany/"
position:
  top: 0
  left: 0
  height: 3
  width: 3
projectName: "the awesome project"
refreshInterval: 300
)",
    },
    {
        "BambooHR",
        "bamboohr",
        "https://wtfutil.com/modules/bamboohr/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 0
  left: 1
  height: 2
  width: 1
refreshInterval: 900
subdomain: "testco"
)",
    },
    {
        "Buildkite",
        "buildkite",
        "https://wtfutil.com/modules/buildkite/",
        R"(
apiKey: "<placeholder>"
enabled: true
organizationSlug: "acme-corp"
refreshInterval: 60
position:
  top: 1
  left: 1
  height: 1
  width: 1
pipelines:
  pipeline-slug-1:
    branches:
      - "master"
      - "stage"
  pipeline-slug-2:
    branches:
      - "master"
      - "production"
)",
    },
    {
        "CircleCI",
        "circleci",
        "https://wtfutil.com/modules/circleci/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 4
  left: 1
  height: 1
  width: 2
refreshInterval: 900
)",
    },
    {
        "Clocks",
        "clocks",
        "https://wtfutil.com/modules/clocks/",
        R"(
colors:
  rows:
    even: "lightblue"
    odd: "white"
enabled: true
locations:
  # From https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
  Avignon: "Europe/Paris"
  Barcelona: "Europe/Madrid"
  Dubai: "Asia/Dubai"
  New York: "America/New York"
  Toronto: "America/Toronto"
  UTC: "Etc/UTC"
  Vancouver: "America/Vancouver"
position:
  top: 4
  left: 0
  height: 1
  width: 1
refreshInterval: 15
# Valid options are: alphabetical, chronological
sort: "alphabetical"
)",
    },
    {
        "CmdRunner",
        "cmdrunner",
        "https://wtfutil.com/modules/cmdrunner/",
        R"(
args: ["-g", "batt"]
cmd: "pmset"
enabled: true
position:
  top: 6
  left: 1
  height: 1
  width: 3
refreshInterval: 30
)",
    },
    {
        "Datadog",
        "datadog",
        "https://wtfutil.com/modules/datadog/",
        R"(
apiKey: "<yourapikey>"
applicationKey: "<yourapplicationkey>"
enabled: true
monitors:
  tags:
    - "team:ops"
position:
  top: 4
  left: 3
  height: 1
  width: 2
)",
    },
    {
        "DEV (dev.to)",
        "devto",
        "https://wtfutil.com/modules/devto/",
        R"(
enabled: true
numberOfArticles: 10
position:
  top: 1
  left: 1
  height: 1
  width: 2
contentTag: "showdev"
contentUsername: "victoravelar"
contentState: "rising"
)",
    },
    {
        "Digital Clock",
        "digitalclock",
        "https://wtfutil.com/modules/digitalclock/",
        R"(
color: orange
enabled: true
font: bigfont
hourFormat: 12
position:
  top: 0
  left: 1
  height: 1
  width: 1
refreshInterval: 1
type: "digitalclock"
)",
    },
    {
        "DigitalOcean",
        "digitalocean",
        "https://wtfutil.com/modules/digitalocean/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 4
  left: 2
  width: 2
  height: 2
refreshInterval: 15
)",
    },
    {
        "Docker",
        "docker",
        "https://wtfutil.com/modules/docker/",
        R"(
enabled: true
labelColor: lightblue
position:
  top: 0
  left: 0
  height: 3
  width: 3
refreshInterval: 1
)",
    },
    {
        "Exchange Rates",
        "exchangerates",
        "https://wtfutil.com/modules/exchange_rates/",
        R"(
enabled: true
focusable: false
position:
  top: 4
  left: 4
  width: 1
  height: 2
rates:
  CAD:
    - "USD"
    - "EUR"
  USD:
    - "CAD"
  EUR:
    - "CAD"
)",
    },
    {
        "Feed Reader",
        "feedreader",
        "https://wtfutil.com/modules/feedreader/",
        R"(
enabled: true
feeds:
- https://news.ycombinator.com/rss
- http://feeds.reuters.com/Reuters/worldNews
feedLimit: 10
position:
  top: 4
  left: 1
  height: 1
  width: 2
refreshInterval: 14400
)",
    },
    {
        "Gerrit",
        "gerrit",
        "https://wtfutil.com/modules/gerrit/",
        R"(
colors:
  rows:
    even: "lightblue"
    odd: "white"
domain: https://gerrit-review.googlesource.com
enabled: true
password: "mypassword"
position:
  top: 2
  left: 3
  height: 2
  width: 2
projects:
  - org/test-project"
  - dotfiles
refreshInterval: 300
username: "myname"
verifyServerCertificate: false
)",
    },
    {
        "Git",
        "git",
        "https://wtfutil.com/modules/git/",
        R"(
commitCount: 5
commitFormat: "[forestgreen]%h [grey]%cd [white]%s [grey]%an[white]"
dateFormat: "%H:%M %d %b %y"
enabled: true
position:
  top: 0
  left: 3
  height: 2
  width: 2
refreshInterval: 8
repositories:
  - "/Users/chris/go/src/github.com/wtfutil/wtf"
  - "/Users/user/fakeapp"
)",
    },
    {
        "Github",
        "github",
        "https://wtfutil.com/modules/github/",
        R"(
apiKey: "<placeholder>"
baseURL: ""
customQueries:
  othersPRs:
    title: "Others Pull Requests"
    filter: "is:open is:pr -author:wtfutil"
enabled: true
enableStatus: true
position:
  top: 2
  left: 3
  height: 2
  width: 2
refreshInterval: 300
repositories:
  - "wtfutil/wtf"
  - "wtfutil/docs"
  - "umbrella-corp/wesker-api"
uploadURL: ""
username: "wtfutil"
)",
    },
    {
        "GitLab",
        "gitlab",
        "https://wtfutil.com/modules/gitlab/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 2
  left: 3
  height: 2
  width: 2
refreshInterval: 300
projects:
  - "gitlab-org/release/tasks"
  - "gitlab-org/gitlab-ce"
username: "wtfutil"
)",
    },
    {
        "Gitter",
        "gitter",
        "https://wtfutil.com/modules/gitter/",
        R"(
apiToken: "<placeholder>"
enabled: true
numberOfMessages: 10
position:
  top: 4
  left: 1
  height: 1
  width: 4
roomUri: wtfutil/Lobby
refreshInterval: 300
)",
    },
    {
        "Hacker News",
        "hackernews",
        "https://wtfutil.com/modules/hackernews/",
        R"(
enabled: true
numberOfStories: 10
position:
  top: 4
  left: 1
  height: 1
  width: 2
storyType: top
refreshInterval: 900
)",
    },
    {
        "Have I Been Pwned",
        "hibp",
        "https://wtfutil.com/modules/hibp/",
        R"(
accounts:
- test@example.com
- pwned@gmail.com
apiKey: "<placeholder>"
colors:
  ok: "green"
  pwned: "red"
enabled: true
position:
  top: 4
  left: 1
  height: 1
  width: 2
refreshInterval: 43200
since: "2019-06-22"
)",
    },
    {
        "IP-API",
        "ipinfo",
        "https://wtfutil.com/modules/ipapi/",
        R"(
colors:
  name: red
  value: white
enabled: true
position:
  top: 1
  left: 2
  height: 1
  width: 1
refreshInterval: 150
)",
    },
    {
        "IPInfo",
        "ipinfo",
        "https://wtfutil.com/modules/ipinfo/",
        R"(
colors:
  name: red
  value: white
enabled: true
position:
  top: 1
  left: 2
  height: 1
  width: 1
refreshInterval: 150
)",
    },
    {
        "Jenkins",
        "jenkins",
        "https://wtfutil.com/modules/jenkins/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 2
  left: 3
  height: 2
  width: 3
refreshInterval: 300
successBallColor: "green"
jobNameRegex: ^[a-z]+\[[0-9]+\]$
url: "https://jenkins.domain.com/jenkins/view_url"
user: "username"
verifyServerCertificate: true
)",
    },
    {
        "Jira",
        "jira",
        "https://wtfutil.com/modules/jira/",
        R"(
apiKey: "<placeholder>"
colors:
  rows:
    even: "lightblue"
    odd: "white"
domain: "https://umbrellacorp.atlassian.net"
email: "chriscummer@me.com"
enabled: true
jql: "issueType = Story"
position:
  top: 4
  left: 1
  height: 1
  width: 2
project: "ProjectA"
refreshInterval: 900
username: "chris.cummer"
verifyServerCertificate: true
)",
    },
    {
        "Kubernetes",
        "kubernetes",
        "https://wtfutil.com/modules/kubernetes/",
        R"(
enabled: true
kubeconfig: "/Users/testuser/.kube/config"
namespaces:
  - internal
  - public
  - systems
objects:
  - deployments
  - nodes
  - pods
position:
  top: 4
  left: 1
  height: 1
  width: 2
refreshInterval: 300
)",
    },
    {
        "Logger",
        "logger",
        "https://wtfutil.com/modules/logger/",
        R"(
enabled: true
position:
  top: 5
  left: 4
  height: 2
  width: 1
refreshInterval: 1
)",
    },
    {
        "Mercurial",
        "mercurial",
        "https://wtfutil.com/modules/mercurial/",
        R"(
commitCount: 5
commitFormat: "[forestgreen]{rev}:{phase} [white]{desc|firstline|strip} [grey]{author|person} {date|age}[white]"
enabled: true
position:
  top: 0
  left: 3
  height: 2
  width: 2
refreshInterval: 8
repositories:
  - "/Users/user/fakelib"
  - "/Users/user/fakeapp"
)",
    },
    {
        "New Relic",
        "newrelic",
        "https://wtfutil.com/modules/newrelic/",
        R"(
apiKey: "<placeholder>"
applicationIDs:
  - 10549735
  - 499785
deployCount: 6
enabled: true
position:
  top: 4
  left: 3
  height: 1
  width: 2
refreshInterval: 900
)",
    },
    {
        "OpsGenie",
        "opsgenie",
        "https://wtfutil.com/modules/opsgenie/",
        R"(
apiKey: "<placeholder>"
displayEmpty: false
enabled: true
position:
  top: 2
  left: 1
  height: 2
  width: 1
refreshInterval: 21600
region: "us"
schedule: "Primary"
scheduleIdentifierType: "id"
)",
    },
    {
        "Pagerduty",
        "pagerduty",
        "https://wtfutil.com/modules/pagerduty/",
        R"(
apiKey: "<placeholder>"
enabled: true
escalationFilter:
- "client-eng"
position:
  top: 4
  left: 3
  height: 1
  width: 2
scheduleIDs:
  - "R2D24CD"
  - "C3P05MF"
showIncidents: true
showSchedules: true
)",
    },
    {
        "Power",
        "power",
        "https://wtfutil.com/modules/power/",
        R"(
enabled: true
position:
  top: 5
  left: 0
  height: 2
  width: 1
refreshInterval: 15
)",
    },
    {
        "Resource Usage",
        "resourceusage",
        "https://wtfutil.com/modules/resourceusage/",
        R"(
cpuCombined: false
enabled: true
position:
  top: 1
  left: 1
  height: 1
  width: 1
refreshInterval: 1
showCPU: true
showMem: true
showSwp: true
)",
    },
    {
        "Rollbar",
        "rollbar",
        "https://wtfutil.com/modules/rollbar/",
        R"(
accessToken: "d23*******************************************3r2"
enabled: true
projectOwner: "ENCOM"
projectName: "MCP"
activeOnly: true
assignedToName: "dillinger"
count: 3
position:
  top: 4
  left: 1
  height: 2
  width: 2
refreshInterval: 900
)",
    },
    {
        "Security",
        "security",
        "https://wtfutil.com/modules/security/",
        R"(
enabled: true
position:
  top: 1
  left: 2
  height: 1
  width: 1
refreshInterval: 3600
)",
    },
    {
        "Football",
        "football",
        "https://wtfutil.com/modules/football/",
        R"(
enabled: true
apiKey: "<placeholder>"
league: "PL"
favTeam: "Liverpool FC"
standingCount: 5
matchesFrom: 5
matchesTo: 5
position:
  top: 1
  left: 0
  height: 4
  width: 3
refreshInterval: 1000
)",
    },
    {
        "NBA Score",
        "nbascore",
        "https://wtfutil.com/modules/nbascore/",
        R"(
enabled: true
position:
  top: 1
  left: 2
  height: 1
  width: 1
refreshInterval: 600
)",
    },
    {
        "Spotify",
        "spotify",
        "https://wtfutil.com/modules/spotify/",
        R"(
enabled: true
colors:
  label: "green"
  text: "white"
position:
  top: 1
  left: 2
  height: 1
  width: 1
refreshInterval: 0
)",
    },
    {
        "Subreddit",
        "subreddit",
        "https://wtfutil.com/modules/subreddit/",
        R"(
enabled: true
numberOfPosts: 10
position:
  top: 4
  left: 1
  height: 1
  width: 2
sortOrder: top
subreddit: "news"
topTimePeriod: month
refreshInterval: 900
)",
    },
    {
        "Textfile",
        "textfile",
        "https://wtfutil.com/modules/textfile/",
        R"(
enabled: true
filePaths:
  - "~/Desktop/notes.md"
  - "~/.config/wtf/config.yml"
format: true
formatStyle: "dracula"
position:
  top: 5
  left: 4
  height: 2
  width: 1
refreshInterval: 15
wrapText: true
)",
    },
    {
        "Todo",
        "todo",
        "https://wtfutil.com/modules/todo/",
        R"(
checkedIcon: "X"
colors:
  checked: gray
  highlight:
    fore: "black"
    back: "orange"
enabled: true
filename: "todo.yml"
position:
  top: 2
  left: 2
  height: 2
  width: 1
refreshInterval: 3600
)",
    },
    {
        "Todoist",
        "todoist",
        "https://wtfutil.com/modules/todoist/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 0
  left: 2
  height: 1
  width: 1
projects:
  - 122247497
refreshInterval: 3600
)",
    },
    {
        "Transmission",
        "transmission",
        "https://wtfutil.com/modules/transmission/",
        R"(
enabled: true
host: "192.168.1.5"
password: "<placeholder>"
position:
  top: 4
  left: 3
  width: 2
  height: 1
refreshInterval: 3
username: "transmission"
)",
    },
    {
        "TravisCI",
        "travisci",
        "https://wtfutil.com/modules/travisci/",
        R"(
apiKey: "<placeholder>"
enabled: true
compact: true
limit: 8
sort_by: "id:desc"
position:
  top: 4
  left: 1
  height: 1
  width: 2
pro: false
refreshInterval: 900
)",
    },
    {
        "Trello",
        "trello",
        "https://wtfutil.com/modules/trello/",
        R"(
accessToken: "<placeholder>"
apiKey: "<placeholder>"
board: Main
enabled: true
list: "Todo"
position:
  height: 1
  left: 2
  top: 0
  width: 1
refreshInterval: 3600
username: myname
)",
    },
    {
        "Twitter Stats",
        "twitterstats",
        "https://wtfutil.com/modules/twitterstats/",
        R"(
consumerKey: "<placeholder>"
consumerSecret: "<placeholder>"
enabled: true
position:
  top: 0
  left: 1
  height: 1
  width: 1
refreshInterval: 20000
screenNames:
  - "wtfutil"
  - "dril"
)",
    },
    {
        "Twitter",
        "twitter",
        "https://wtfutil.com/modules/twittertweets/",
        R"(
bearerToken: "<placeholder>"
count: 5
enabled: true
position:
  top: 0
  left: 1
  height: 1
  width: 1
refreshInterval: 20000
screenName: "wtfutil"
)",
    },
    {
        "VictorOps OnCall",
        "victorops",
        "https://wtfutil.com/modules/victorops/",
        R"(
apiID: a3c5dd63
apiKey: "<placeholder>"
enabled: true
position:
  top: 0
  left: 1
  height: 2
  width: 1
refreshInterval: 3600
team: devops
)",
    },
    {
        "ARPANSA",
        "arpansagovau",
        "https://wtfutil.com/modules/arpansagovau/",
        R"(
enabled: true
locationid: "Sydney"
refreshInterval: 900
position:
  top: 0
  left: 0
  height: 1
  width: 1
)",
    },
    {
        "Pretty Weather",
        "prettyweather",
        "https://wtfutil.com/modules/prettyweather/",
        R"(
enabled: true
city: "tehran"
position:
  top: 3
  left: 5
  height: 1
  width: 1
refreshInterval: 300
unit: "m"
view: 0
language: "en"
)",
    },
    {
        "Weather",
        "weather",
        "https://wtfutil.com/modules/weather/",
        R"(
apiKey: "<placeholder>"
# From http://openweathermap.org/help/city_list.txt
cityids:
- 6173331
- 3128760
- 6167865
- 6176823
colors:
  current: "lightblue"
enabled: true
language: "EN"
position:
  top: 0
  left: 2
  height: 1
  width: 1
refreshInterval: 900
tempUnit: "C"
)",
    },
    {
        "Zendesk",
        "zendesk",
        "https://wtfutil.com/modules/zendesk/",
        R"(
apiKey: "<placeholder>"
enabled: true
position:
  top: 0
  left: 2
  height: 1
  width: 1
status: "new"
subdomain: "your_domain"
username: "your_email@acme.com"
)",
    },
};

}
