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
  
  return {
    'stdout': textSummary(data, { indent: ' ', enableColors: true }), 
    'stderr': textSummary(data, { indent: ' ', enableColors: true, onlyFailed: true }),
    './loadtest-result.xml': jUnit(data), 
  };
}

// 'stderr': textSummary(data, { indent: ' ', enableColors: true, onlyFailed: true })
