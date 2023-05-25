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


