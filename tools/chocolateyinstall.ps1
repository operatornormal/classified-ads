
$ErrorActionPreference = 'Stop'; # stop on all errors
$packageName = 'classified-ads' 
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"

$url = 'https://github.com/operatornormal/classified-ads/raw/8fa7328df30a54203313f045e50fe7ea5375e99a/Classified-ads-Win32.exe' # download url

$packageArgs = @{
  packageName   = $packageName
  unzipLocation = $toolsDir
  fileType      = 'exe' 
  url           = $url

  #MSI
  silentArgs    = "/S" 
  validExitCodes= @(0, 3010, 1641)
  registryUninstallerKey = 'classified-ads' #ensure this is the value in the registry
  checksum      = 'ab0e44d0899a089b54d988f18c02829935535263'
  checksumType  = 'sha1' #default is md5, can also be sha1
}

Install-ChocolateyPackage @packageArgs
