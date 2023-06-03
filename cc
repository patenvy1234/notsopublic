from flask import Flask, render_template, request, jsonify
from azure.storage.blob import BlobServiceClient
import json

app = Flask(__name__)

# Azure Blob Storage credentials
connection_string = '<your_blob_connection_string>'
container_name = '<your_container_name>'

# Initialize Azure Blob Service
blob_service_client = BlobServiceClient.from_connection_string(connection_string)
container_client = blob_service_client.get_container_client(container_name)

# Get the difference in http_req_duration avg between two JSON files
def get_duration_diff(file1_data, file2_data):
    avg1 = file1_data['metrics']['http_req_duration']['values']['avg']
    avg2 = file2_data['metrics']['http_req_duration']['values']['avg']
    diff = avg1 - avg2
    return diff

# Route for the home page
@app.route('/', methods=['GET'])
def home():
    return render_template('index.html')

# Route for processing the search request
@app.route('/search', methods=['POST'])
def search():
    file1_name = request.form['file1']
    file2_name = request.form['file2']
    
    # Retrieve file contents from Azure Blob storage
    file1_blob_client = container_client.get_blob_client(file1_name)
    file1_content = file1_blob_client.download_blob().readall().decode('utf-8')
    file1_data = json.loads(file1_content)
    
    file2_blob_client = container_client.get_blob_client(file2_name)
    file2_content = file2_blob_client.download_blob().readall().decode('utf-8')
    file2_data = json.loads(file2_content)
    
    # Calculate the difference in http_req_duration avg
    diff = get_duration_diff(file1_data, file2_data)
    
    # Return the difference as JSON response
    response = {
        'file1': file1_name,
        'file2': file2_name,
        'duration_diff': diff
    }
    return jsonify(response)

if __name__ == '__main__':
    app.run()






<!DOCTYPE html>
<html>
  <head>
    <title>File Search</title>
  </head>
  <body>
    <h1>File Search</h1>
    <form method="post" action="/search">
      <label for="file1">Select File 1:</label>
      <select name="file1">
        {% for file in files %}
          <option value="{{ file }}">{{ file }}</option>
        {% endfor %}
      </select>
      <br>
      <label for="file2">Select File 2:</label>
      <select name="file2">
        {% for file in files %}
          <option value="{{ file }}">{{ file }}</option>
        {% endfor %}
      </select>
      <br>
      <button type="submit">Search</button>
    </form>
    <div id="result"></div>
    
    <script>
      // Handle form submission with AJAX
      document.querySelector('form').addEventListener('submit', function(event) {
        event.preventDefault();
        var form = event.target;
        var url = form.action;
        var formData = new FormData(form);
        
        fetch(url, {
          method: 'POST',
          body: formData
        })
        .then(function(response) {
          return response.json();
        })
        .then(function(data) {
          displayResult(data);
        })
        .catch(function(error) {
          console.error('Error:', error);
        });
      });

      // Function to display the result
      function displayResult(data) {
        var resultDiv = document.getElementById('result');
        resultDiv.innerHTML = 'Difference in http_req_duration avg: ' + data.duration_diff;
      }
    </script>
  </body>
</html>



















from flask import Flask, render_template, request
from azure.storage.blob import BlobServiceClient

app = Flask(__name__)

# Azure Blob Storage credentials
connection_string = '<your_blob_connection_string>'
container_name = '<your_container_name>'

# Initialize Azure Blob Service
blob_service_client = BlobServiceClient.from_connection_string(connection_string)
container_client = blob_service_client.get_container_client(container_name)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        selected_file = request.form['selected_file']
        data = get_blob_contents(selected_file)
        return render_template('index.html', data=data, selected_file=selected_file)
    blob_names = get_blob_names()
    return render_template('index.html', blob_names=blob_names)

def get_blob_names():
    blob_names = []
    for blob in container_client.list_blobs():
        if blob.name.endswith('.json'):
            blob_names.append(blob.name)
    return blob_names

def get_blob_contents(blob_name):
    blob_client = container_client.get_blob_client(blob_name)
    blob_data = blob_client.download_blob().readall()
    return blob_data.decode('utf-8')

if __name__ == '__main__':
    app.run()






<!DOCTYPE html>
<html>
  <head>
    <title>File Search</title>
    <style>
      ul {
        list-style-type: none;
        padding: 0;
      }
      li {
        margin-bottom: 10px;
        padding: 10px;
        border: 1px solid #ccc;
        border-radius: 5px;
      }
      .title {
        font-weight: bold;
        margin-bottom: 5px;
      }
      .content {
        white-space: pre-wrap;
      }
    </style>
  </head>
  <body>
    <h1>File Search</h1>
    <form method="post">
      <select name="selected_file">
        {% for blob_name in blob_names %}
          <option value="{{ blob_name }}">{{ blob_name }}</option>
        {% endfor %}
      </select>
      <button type="submit">Search</button>
    </form>
    {% if data %}
      <h2>File: {{ selected_file }}</h2>
      <pre>{{ data }}</pre>
    {% else %}
      <p>No file selected.</p>
    {% endif %}
  </body>
</html>













from flask import Flask, render_template, request
from azure.storage.blob import BlobServiceClient
import json

app = Flask(__name__)

# Azure Blob Storage credentials
connection_string = '<your_blob_connection_string>'
container_name = '<your_container_name>'

# Initialize Azure Blob Service
blob_service_client = BlobServiceClient.from_connection_string(connection_string)
container_client = blob_service_client.get_container_client(container_name)

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        search_term = request.form['search']
        blobs = search_blobs(search_term)
        data = get_blob_contents(blobs)
        return render_template('index.html', data=data)
    return render_template('index.html', data=None)

def search_blobs(search_term):
    blob_names = []
    for blob in container_client.list_blobs():
        if search_term.lower() in blob.name.lower():
            blob_names.append(blob.name)
    return blob_names

def get_blob_contents(blob_names):
    data = []
    for name in blob_names:
        blob_client = container_client.get_blob_client(name)
        blob_data = blob_client.download_blob().readall()
        json_data = json.loads(blob_data)
        data.append(json_data)
    return data

if __name__ == '__main__':
    app.run()


<!DOCTYPE html>
<html>
  <head>
    <title>File Search</title>
    <style>
      ul {
        list-style-type: none;
        padding: 0;
      }
      li {
        margin-bottom: 10px;
        padding: 10px;
        border: 1px solid #ccc;
        border-radius: 5px;
      }
      .title {
        font-weight: bold;
        margin-bottom: 5px;
      }
      .content {
        white-space: pre-wrap;
      }
    </style>
  </head>
  <body>
    <h1>File Search</h1>
    <form method="post">
      <input type="text" name="search" placeholder="Search files" required>
      <button type="submit">Search</button>
    </form>
    {% if data %}
      <h2>Search Results:</h2>
      {% for item in data %}
        <li>
          <div class="title">{{ item['title'] }}</div>
          <div class="content">{{ item['content'] }}</div>
        </li>
      {% endfor %}
    {% else %}
      <p>No results found.</p>
    {% endif %}
  </body>
</html>





#!/bin/bash

json_file="path/to/your/file.json"

# Read http_req_duration average value
avg=$(jq -r '.metrics.http_req_duration.values.avg' "$json_file")

echo "Average http_req_duration: $avg"




#!/bin/bash

# Install k6
echo "Installing k6..."
curl -sSL https://install.k6.io | bash
export PATH="$HOME/.k6/bin:$PATH"

# Run script1.js and script2.js in parallel
echo "Running scripts in parallel..."
k6 run script1.js &
k6 run script2.js &

# Wait for both scripts to finish
wait

# Compare the results
echo "Comparing the results..."
# Add your comparison logic here

echo "Script execution completed."






# Define the variables to compare
$variable1 = 10
$variable2 = 15
$variable3 = 20
$variable4 = 25
$variable5 = 30
$variable6 = 35
$variable7 = 40
$variable8 = 45
$variable9 = 50
$variable10 = 55

# Define the range for comparison
$range = 5

# Create two arrays with the variables to compare
$firstSet = $variable1, $variable2, $variable3, $variable4, $variable5
$secondSet = $variable6, $variable7, $variable8, $variable9, $variable10

# Generate the output
$output = ""
for ($i = 0; $i -lt $firstSet.Count; $i++) {
    $diff = [Math]::Abs($firstSet[$i] - $secondSet[$i])
    $isInRange = $diff -lt $range
    $output += "╔════════════╦════════════╦════════════════════╗`n"
    $output += "║ Variable $($i+1)  ║ Variable $($i+6)  ║ Difference (±$range) ║`n"
    $output += "╠════════════╬════════════╬════════════════════╣`n"
    $output += "║ $($firstSet[$i])      ║ $($secondSet[$i])      ║ $($diff.ToString())                  "
    if ($isInRange) {
        $output += "√"
    }
    else {
        $output += "X"
    }
    $output += "`n"
    $output += "╚════════════╩════════════╩════════════════════╝`n"
}

# Output the result
$output







# Variables to compare
$variable1 = "$(Variable1)" # Replace with the actual variable name in ADO
$variable2 = "$(Variable2)" # Replace with the actual variable name in ADO

# Perform comparison logic
$comparisonResult = if ($variable1 -eq $variable2) { "Variables are equal" } else { "Variables are not equal" }

# Generate HTML report
$htmlContent = @"
<!DOCTYPE html>
<html>
<head>
    <title>Variable Comparison Report</title>
</head>
<body>
    <h1>Variable Comparison Report</h1>
    <table>
        <tr>
            <th>Variable 1</th>
            <th>Variable 2</th>
            <th>Comparison Result</th>
        </tr>
        <tr>
            <td>$variable1</td>
            <td>$variable2</td>
            <td>$comparisonResult</td>
        </tr>
    </table>
</body>
</html>
"@

# Save HTML report to a file
$reportFilePath = "variable_comparison_report.html"
$htmlContent | Out-File -FilePath $reportFilePath -Encoding UTF8

# Output the report file path for reference
Write-Host "Variable comparison report generated: $reportFilePath"









Hello Everyone,

Allow me to introduce myself as the latest addition to our remarkable Azure Front Door team, joining you all as an intern.

My name is Pathik Patel and I am currently pursuing Maths and Computing at DAIICT. I have a diverse range of interests that keep me engaged outside of work. I have particular interest in Chess, Football and F1 also a great fan of Anime also I am open to feedback and always strive for continuous improvement.

As an intern, I am excited to collaborate, contribute, and grow alongside all of you and also seek the valuable inputs and guidance of my mentors and managers, as their insights will be instrumental in shaping myself as a person.


# Install required PowerShell modules
Install-Module -Name Az -Force -AllowClobber

# Connect to your Azure DevOps account
Connect-AzAccount

# Specify the Azure DevOps organization, project, and deployment job group details
$orgUrl = "https://dev.azure.com/YourOrganization"
$projectName = "YourProject"
$deploymentJobGroup = "YourDeploymentJobGroup"

# Specify the deployment job name
$deploymentJobName = "YourDeploymentJob"

# Get the deployment job details
$jobDetails = Get-AzDevOpsArtifact -Organization $orgUrl -Project $projectName -Name $deploymentJobGroup -Type JobGroup

# Get the deployment job ID
$jobId = $jobDetails.id

# Get the deployment job group instance ID
$instanceId = $jobDetails.resource.data.information.deployments | Where-Object { $_.name -eq $deploymentJobName } | Select-Object -ExpandProperty id

# Update the deployment job instance status to completed
$jobStatusUrl = "$orgUrl/$projectName/_apis/distributedtask/hubs/jobResults/$jobId/$instanceId?api-version=6.0-preview.1"
$jobStatusBody = @{
    status = "completed"
} | ConvertTo-Json

Invoke-AzRestMethod -Method Patch -Uri $jobStatusUrl -Body $jobStatusBody

# Set the desired state of the VMs to turned off
$desiredStateUrl = "$orgUrl/$projectName/_apis/distributedtask/hubs/jobRequests/$jobId/$instanceId/properties?api-version=6.0-preview.1"
$desiredStateBody = @{
    desiredState = "2" # 2 indicates 'TurnedOff' state
} | ConvertTo-Json

Invoke-AzRestMethod -Method Patch -Uri $desiredStateUrl -Body $desiredStateBody





# Install-Module -Name UniversalDashboard if the module is not already installed

Import-Module -Name UniversalDashboard

# Set the response times for Build A and Build B
$buildAResponseTime = 10.5 # Replace with the actual response time for Build A
$buildBResponseTime = 8.2  # Replace with the actual response time for Build B

# Create a new dashboard
$dashboard = New-UDDashboard -Title "Response Time Dashboard" -Content {
    New-UDRow -Columns {
        New-UDColumn -Size 6 -Content {
            New-UDCard -Title "Build A" -Content {
                New-UDTypography -Text "Response Time: $buildAResponseTime ms"
            }
        }
        New-UDColumn -Size 6 -Content {
            New-UDCard -Title "Build B" -Content {
                New-UDTypography -Text "Response Time: $buildBResponseTime ms"
            }
        }
    }
}

# Start the dashboard server
Start-UDDashboard -Port 8080 -Dashboard $dashboard -Wait


Install-Module -Name VSTeam -Force

$pat = "Personal Access Token"  # Replace with your own Personal Access Token
$orgUrl = "https://dev.azure.com/YourOrganizationName"  # Replace with your Azure DevOps organization URL
$project = "YourProjectName"  # Replace with your Azure DevOps project name

# Authenticate with Azure DevOps using Personal Access Token
Connect-VSTeamAccount -AccessToken $pat -AccountUrl $orgUrl

# Define the variable name and value
$variableName = "MyVariable"
$variableValue = "MyValue"

# Set the variable in the pipeline
Set-VSTeamVariable -ProjectName $project -VariableName $variableName -VariableValue $variableValue

$jsonString = $lkg -replace '\r?\n','' -replace '\s+'

# Assuming you have a JSON string in the variable $jsonString
jsonObject=$(jq --arg jsonString "$jsonString" -n '$jsonString | fromjson')

Write-Host "##vso[task.setvariable variable=PipelineVariableName]VariableValue"

sonObject=$(echo "$jsonString" | jq '.')

jsonObject=$(echo "$jsonString" | jq --arg jsonString "$jsonString" '. as $json | $json | fromjson')

# Convert JSON string to JSON object
$jsonObject = ConvertFrom-Json -InputObject $jsonString


{
  "state": {
    "isStdOutTTY": false,
    "isStdErrTTY": false,
    "testRunDurationMs": 20050.69906
  },
  "metrics": {
    "iteration_duration": {
      "type": "trend",
      "contains": "time",
      "values": "@{max=924.276062; p(90)=308.953249; p(95)=476.9545769; avg=124.71312956083; min=4.518524; med=82.864757}"
    },
    "http_req_tls_handshaking": {
      "type": "trend",
      "contains": "time",
      "values": "@{min=0; med=0; max=13.257341; p(90)=0; p(95)=0; avg=0.106053631830874}"
    },
    "http_req_receiving": {
      "type": "trend",
      "contains": "time",
      "values": "@{avg=0.0443709912245713; min=0.016211; med=0.032357; max=25.730234; p(90)=0.0445018; p(95)=0.0492848}"
    },
    "http_reqs": {
      "type": "counter",
      "contains": "default",
      "values": "@{count=2507; rate=125.033047102149}"
    },
    "http_req_failed": {
      "type": "rate",
      "contains": "default",
      "values": "@{rate=0; passes=0; fails=2507}"
    },
    "vus": {
      "type": "gauge",
      "contains": "default",
      "values": "@{value=6; min=2; max=29}"
    },
    "http_req_blocked": {
      "type": "trend",
      "contains": "time",
      "values": "@{min=0.000821; med=0.00147; max=31.850221; p(90)=0.0021102; p(95)=0.0023797; avg=0.142989405265257}"
    },
    "vus_max": {
      "contains": "default",
      "values": "@{value=30; min=30; max=30}",
      "type": "gauge"
    },
    "http_req_waiting": {
      "type": "trend",
      "contains": "time",
      "values": "@{med=82.114991; max=923.574214; p(90)=308.2357396; p(95)=476.2938964; avg=123.733731508576; min=3.773699}"
    },
    "http_req_connecting": {
      "type": "trend",
      "contains": "time",
      "values": "@{avg=0.0268021208615876; min=0; med=0; max=13.677725; p(90)=0; p(95)=0}"
    },
    "data_received": {
      "type": "counter",
      "contains": "data",
      "values": "@{count=9902248; rate=493860.486877209}"
    },
    "data_sent": {
      "values": "@{count=334606; rate=16687.9967126692}",
      "type": "counter",
      "contains": "data"
    },
    "http_req_duration": {
      "values": "@{min=3.819621; med=82.145855; max=923.616542; p(90)=308.275711; p(95)=476.3350163; avg=123.789769893099}",
      "thresholds": "@{p(95)<4000=}",
      "type": "trend",
      "contains": "time"
    },
    "http_req_sending": {
      "values": "@{p(95)=0.0107638; avg=0.0116673932987635; min=0.00423; med=0.006849; max=10.454438; p(90)=0.0096482}",
      "type": "trend",
      "contains": "time"
    },
    "http_req_duration{expected_response:true}": {
      "contains": "time",
      "values": "@{min=3.819621; med=82.145855; max=923.616542; p(90)=308.275711; p(95)=476.3350163; avg=123.789769893099}",
      "type": "trend"
    },
    "checks": {
      "type": "rate",
      "contains": "default",
      "values": "@{passes=2507; fails=0; rate=1}"
    },
    "iterations": {
      "type": "counter",
      "contains": "default",
      "values": "@{count=2507; rate=125.033047102149}"
    }
  },
  "root_group": {
    "name": "",
    "path": "",
    "id": "d41d8cd98f00b204e9800998ecf8427e",
    "groups": [],
    "checks": [
      "@{path=::is status 200; id=548d37ca5f33793206f7832e7cea54fb; passes=2507; fails=0; name=is status 200}"
    ]
  },
  "options": {
    "summaryTrendStats": [
      "avg",
      "min",
      "med",
      "max",
      "p(90)",
      "p(95)"
    ],
    "summaryTimeUnit": "",
    "noColor": false
  }
}
{
  "state": {
    "isStdOutTTY": false,
    "isStdErrTTY": false,
    "testRunDurationMs": 20050.69906
  },
  "metrics": {
    "iteration_duration": {
      "type": "trend",
      "contains": "time",
      "values": "@{max=924.276062; p(90)=308.953249; p(95)=476.9545769; avg=124.71312956083; min=4.518524; med=82.864757}"
    },
    "http_req_tls_handshaking": {
      "type": "trend",
      "contains": "time",
      "values": "@{min=0; med=0; max=13.257341; p(90)=0; p(95)=0; avg=0.106053631830874}"
    },
    "http_req_receiving": {
      "type": "trend",
      "contains": "time",
      "values": "@{avg=0.0443709912245713; min=0.016211; med=0.032357; max=25.730234; p(90)=0.0445018; p(95)=0.0492848}"
    },
    "http_reqs": {
      "type": "counter",
      "contains": "default",
      "values": "@{count=2507; rate=125.033047102149}"
    },
    "http_req_failed": {
      "type": "rate",
      "contains": "default",
      "values": "@{rate=0; passes=0; fails=2507}"
    },
    "vus": {
      "type": "gauge",
      "contains": "default",
      "values": "@{value=6; min=2; max=29}"
    },
    "http_req_blocked": {
      "type": "trend",
      "contains": "time",
      "values": "@{min=0.000821; med=0.00147; max=31.850221; p(90)=0.0021102; p(95)=0.0023797; avg=0.142989405265257}"
    },
    "vus_max": {
      "contains": "default",
      "values": "@{value=30; min=30; max=30}",
      "type": "gauge"
    },
    "http_req_waiting": {
      "type": "trend",
      "contains": "time",
      "values": "@{med=82.114991; max=923.574214; p(90)=308.2357396; p(95)=476.2938964; avg=123.733731508576; min=3.773699}"
    },
    "http_req_connecting": {
      "type": "trend",
      "contains": "time",
      "values": "@{avg=0.0268021208615876; min=0; med=0; max=13.677725; p(90)=0; p(95)=0}"
    },
    "data_received": {
      "type": "counter",
      "contains": "data",
      "values": "@{count=9902248; rate=493860.486877209}"
    },
    "data_sent": {
      "values": "@{count=334606; rate=16687.9967126692}",
      "type": "counter",
      "contains": "data"
    },
    "http_req_duration": {
      "values": "@{min=3.819621; med=82.145855; max=923.616542; p(90)=308.275711; p(95)=476.3350163; avg=123.789769893099}",
      "thresholds": "@{p(95)<4000=}",
      "type": "trend",
      "contains": "time"
    },
    "http_req_sending": {
      "values": "@{p(95)=0.0107638; avg=0.0116673932987635; min=0.00423; med=0.006849; max=10.454438; p(90)=0.0096482}",
      "type": "trend",
      "contains": "time"
    },
    "http_req_duration{expected_response:true}": {
      "contains": "time",
      "values": "@{min=3.819621; med=82.145855; max=923.616542; p(90)=308.275711; p(95)=476.3350163; avg=123.789769893099}",
      "type": "trend"
    },
    "checks": {
      "type": "rate",
      "contains": "default",
      "values": "@{passes=2507; fails=0; rate=1}"
    },
    "iterations": {
      "type": "counter",
      "contains": "default",
      "values": "@{count=2507; rate=125.033047102149}"
    }
  },
  "root_group": {
    "name": "",
    "path": "",
    "id": "d41d8cd98f00b204e9800998ecf8427e",
    "groups": [],
    "checks": [
      "@{path=::is status 200; id=548d37ca5f33793206f7832e7cea54fb; passes=2507; fails=0; name=is status 200}"
    ]
  },
  "options": {
    "summaryTrendStats": [
      "avg",
      "min",
      "med",
      "max",
      "p(90)",
      "p(95)"
    ],
    "summaryTimeUnit": "",
    "noColor": false
  }
}


kubectl run k6 --image=k6io/k6 --restart=Never -- --help

kubectl exec -it <pod_name> -- k6 run <test_script.js>

curl -O https://dl.google.com/go/goq.20.4.linux-amd64.tar.gz \
&& sudo tar -xvf go1.20.4.linux-amd64.tar.gz -C /usr/local \
&& echo "export PATH=\$PATH:/usr/local/go/bin" >> ~/.bashrc \
&& echo "export GOPATH=\$HOME/go" >> ~/.bashrc \
&& echo "export PATH=\$PATH:\$GOPATH/bin" >> ~/.bashrc \
&& source ~/.bashrc

curl -O https://dl.google.com/go/go1.20.4.linux-amd64.tar.gz && sudo tar -xvf go1.20.4.linux-amd64.tar.gz -C /usr/local


curl -O https://dl.google.com/go/go<VERSION>.linux-amd64.tar.gz && sudo tar -xvf go<VERSION>.linux-amd64.tar.gz -C /usr/local




 rm -rf /usr/local/go && tar -C /usr/local -xzf go1.20.4.linux-amd64.tar.gz



GET https://dev.azure.com/{organization}/{project}/_apis/distributedtask/variablegroups?groupName={groupName}&api-version=6.1-preview.2

top -b -n 1 | grep "Cpu(s)" | awk '{print $2 + $4}'
sar -u 1 10 | awk 'NR>3 {print 100 - $NF}'
sudo apt-get install sysstat -y

top -b -n 1 -d 1 | awk '/^%Cpu/ {if ($2+$4 > max) max=$2+$4} END {print max}'


sar -u 1 1 | tail -n 1 | awk '{print 100 - $NF}'

curl -X PATCH \
  -H "Authorization: Bearer <personal-access-token>" \
  -H "Content-Type: application/json-patch+json" \
  --data '[{
    "op": "add",
    "path": "/variables/Variable1",
    "value": {
      "value": "UpdatedValue1"
    }
  },{
    "op": "add",
    "path": "/variables/Variable2",
    "value": {
      "value": "UpdatedValue2"
    }
  }]' \
  "https://dev.azure.com/{organization}/{project}/_apis/distributedtask/variablegroups/{groupId}?api-version=6.1-preview.2"


curl -X GET \
  -H "Authorization: Bearer {PAT}" \
  "https://dev.azure.com/{organization}/{project}/_apis/distributedtask/variablegroups?groupName={groupName}&api-version=6.1-preview.2"


curl -X PATCH \
  -H "Authorization: Bearer {PAT}" \
  -H "Content-Type: application/json-patch+json" \
  -d '[{
    "op": "add",
    "path": "/variables/Variable1",
    "value": {
      "value": "UpdatedValue1"
    }
  },{
    "op": "add",
    "path": "/variables/Variable2",
    "value": {
      "value": "UpdatedValue2"
    }
  }]' \
  "https://dev.azure.com/{organization}/{project}/_apis/distributedtask/variablegroups/{groupId}?api-version=6.1-preview.2"





trigger:
  branches:
    include:
      - main  # Replace with your branch name

pool:
  vmImage: 'ubuntu-latest'

steps:
- script: |
    echo "Building your code..."
    # Add build commands here
  displayName: 'Build'

- task: CopyFiles@2
  inputs:
    sourceFolder: '$(System.DefaultWorkingDirectory)'
    targetFolder: '$(Build.ArtifactStagingDirectory)'
    contents: '**'
    flattenFolders: true

- task: PublishBuildArtifacts@1
  inputs:
    pathtoPublish: '$(Build.ArtifactStagingDirectory)'
    artifactName: 'BuildArtifacts'
    publishLocation: 'Container'
    
    
    
    -i -v $(Pipeline.Workspace):/src loadtests/local.j
    
    
    
    
    
    trigger:
  branches:
    include:
      - main  # Replace with your branch name

pool:
  vmImage: 'ubuntu-latest'

steps:
- task: Docker@2
  displayName: 'Pull k6 Docker image'
  inputs:
    command: 'pull'
    containerRegistry: 'YourContainerRegistryConnection'
    imageName: 'loadimpact/k6'

- task: Docker@2
  displayName: 'Create k6 container'
  inputs:
    command: 'create'
    containerRegistry: 'YourContainerRegistryConnection'
    imageName: 'loadimpact/k6'
    containerName: 'k6-container'

- task: Docker@2
  displayName: 'Copy k6 binary'
  inputs:
    command: 'cp'
    containerRegistry: 'YourContainerRegistryConnection'
    containerName: 'k6-container'
    source: ':/usr/bin/k6'
    target: '$(Build.ArtifactStagingDirectory)'

- task: PublishBuildArtifacts@1
  displayName: 'Publish k6 artifact'
  inputs:
    pathtoPublish: '$(Build.ArtifactStagingDirectory)'
    artifactName: 'k6'
    publishLocation: 'Container'


import http from 'k6/http';
import { check, fail } from 'k6';
import { jUnit, textSummary } from 'https://jslib.k6.io/k6-summary/0.0.1/index.js';

export const options = {
  stages: [
    { duration: '10s', target: 10 },
    { duration: '20s' },
    { duration: '10s', target: 5},
  ],
  thresholds: {
    http_req_duration: ['p(95)<250'],
    // Add other thresholds as needed
  },
};

export default function () {
  let res = http.get(`${__ENV.API_PROTOCOL}://${__ENV.API_BASEURL}/Product/GetAllProducts`);
  
  // Check response status
  check(res, {
    'is status 200': (r) => r.status === 200,
  });
  
  // Check threshold limits
  check(res, {
    'response duration is within threshold': (r) => r.timings.duration <= 250,
    // Add other threshold checks as needed
  }) || fail('Threshold limit breached');
}

export function handleSummary(data) {
  let filepath = `./${__ENV.TESTRESULT_FILENAME}-result.xml`;

  // Include threshold failures in the summary report
  const thresholdBreaches = Object.entries(data.thresholds).map(([metric, { breaches }]) => {
    return breaches.map((b) => {
      return {
        name: `${metric} ${b.threshold}`,
        message: `${b.metric} ${b.threshold} ${b.operator} ${b.value}`,
        type: 'failure',
      };
    });
  }).flat();

  data.errors.push(...thresholdBreaches);
  
  
  
  #!/bin/bash

# Read the contents of the first JSON file
file1=$(cat file1.json)

# Read the contents of the second JSON file
file2=$(cat file2.json)

# Extract the http_req_duration average from both files
avg1=$(echo "$file1" | jq -r '.metrics.http_req_duration.values.avg')
avg2=$(echo "$file2" | jq -r '.metrics.http_req_duration.values.avg')

# Calculate the percentage degradation
percentage_degradation=$(bc <<< "scale=2; (($avg2 - $avg1) / $avg1) * 100")

# Output the percentage degradation
echo "Percentage Degradation in http_req_duration average: $percentage_degradation%"

  
  return {
    'stdout': textSummary(data, { indent: ' ', enableColors: true }), 
    'stderr': textSummary(data, { indent: ' ', enableColors: true, onlyFailed: true }),
    './loadtest-result.xml': jUnit(data), 
  };
}

// 'stderr': textSummary(data, { indent: ' ', enableColors: true, onlyFailed: true })



 $filePath = '<path_to_json_file>'  # Replace with the actual path to your JSON file
          $jsonData = Get-Content -Path $filePath -Raw | ConvertFrom-Json
          $httpReqDurationAvg = $jsonData.metrics.http_req_duration.avg

          Write-Host "##vso[task.setvariable variable=HttpReqDurationAvg;isOutput=true]$httpReqDurationAvg"

      displayName: 'Read JSON File and Save Variable'

    - script: echo $(HttpReqDurationAvg)
      displayName: 'Print Variable Value'

Get-ChildItem -File -Recurse | ForEach-Object {
    Write-Host $_.FullName
}



$jsonFilePath = "C:\path\to\your\json\file.json"
$jsonContent = Get-Content -Path $jsonFilePath -Raw | ConvertFrom-Json
$jsonContent | Format-Table -AutoSize   # Display as formatted table
# or
$jsonContent | ConvertTo-Json            # Display as JSON


jsonData=$(cat /path/to/your/file.json)
httpReqDurationAvg=$(echo "$jsonData" | jq -r '.metrics.http_req_duration.avg')
echo "##vso[task.setvariable variable=HttpReqDurationAvg;isOutput=true]$httpReqDurationAvg"


GET https://vsrm.dev.azure.com/<organization>/<project>/_apis/release/releases/<releaseId>?api-version=6.0
GET https://vsrm.dev.azure.com/<organization>/<project>/_apis/release/releases/<releaseId>?api-version=6.0


$variable1 = $env:VARIABLE1
$variable2 = $env:VARIABLE2

# Calculate the relative difference
$relativeDifference = ($variable2 - $variable1) / $variable1 * 100

# Check if the relative difference exceeds 5%
if ($relativeDifference -gt 5) {
    Write-Host "Relative difference ($relativeDifference%) is greater than 5%. Failing the pipeline."
    exit 1
} else {
    Write-Host "Relative difference ($relativeDifference%) is within the acceptable range."
}

variable1=$VARIABLE1
variable2=$VARIABLE2

# Calculate the relative difference
relativeDifference=$(echo "scale=2; (($variable2 - $variable1) / $variable1) * 100" | bc)

# Check if the relative difference exceeds 5%
if (( $(echo "$relativeDifference > 5" | bc -l) )); then
    echo "Relative difference ($relativeDifference%) is greater than 5%. Failing the pipeline."
    exit 1
else
    echo "Relative difference ($relativeDifference%) is within the acceptable range."
fi


echo "##vso[task.logissue type=error]Relative difference ($relativeDifference%) is greater than 5%. Failing the pipeline."
    exit 1
    
    
    
    
    echo $SYSTEM_ACCESSTOKEN | az devops login
      az pipelines variable-group variable update --group-id $(group_id) \
        --name NAME_OF_THE_VARIABLE \
        --value "${newValue}" \
        --org https://dev.azure.com/YOUR_ORGANIZATION_NAME \
        --project AZURE_DEVOPS_PROJECT




scenarios: {
    contacts: {
      executor: 'ramping-arrival-rate',

      // Start iterations per `timeUnit`
      startRate: 0,

      // Start `startRate` iterations per minute
      timeUnit: '1s',

      // Pre-allocate necessary VUs.
      preAllocatedVUs: 0,
      maxVUs : 1000,


      stages: [
   
        // Linearly ramp-up to starting 600 iterations per `timeUnit` over the following two minutes.
        { target: 100, duration: '5s' },

        // Cntinue starting 600 iterations per `timeUnit` for the following four minutes.
        { target: 100, duration: '10s' },

        // Linearly ramp-down to starting 60 iterations per `timeUnit` over the last two minute.
        { target: 0, duration: '5s' },
      ],
    },
  },
