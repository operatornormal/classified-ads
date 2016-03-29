
$ErrorActionPreference = 'Stop'; # stop on all errors
$packageName = 'classified-ads' 
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"

$url = 'https://github.com/operatornormal/classified-ads/raw/fbc222df590a3a7ea2014620ac54e87c79ba96e6/Classified-ads-Win32.exe' # download url

$packageArgs = @{
  packageName   = $packageName
  unzipLocation = $toolsDir
  fileType      = 'exe' 
  url           = $url

  #MSI
  silentArgs    = "/S" 
  validExitCodes= @(0, 3010, 1641)
  registryUninstallerKey = 'classified-ads' #ensure this is the value in the registry
  checksum      = 'ec7c825f6121a1268f904510bd67b028230e2f00'
  checksumType  = 'sha1' #default is md5, can also be sha1
}

Install-ChocolateyPackage @packageArgs
