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

