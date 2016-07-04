
$ErrorActionPreference = 'Stop'; # stop on all errors
$packageName = 'classified-ads' 
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"

$url = 'https://github.com/operatornormal/classified-ads/raw/df67d1b7dfff90916e3e1e527bbef404c806b290/Classified-ads-Win32.exe' # download url

$packageArgs = @{
  packageName   = $packageName
  unzipLocation = $toolsDir
  fileType      = 'exe' 
  url           = $url

  #MSI
  silentArgs    = "/S" 
  validExitCodes= @(0, 3010, 1641)
  registryUninstallerKey = 'classified-ads' #ensure this is the value in the registry
  checksum      = '75963c8923e8c8949f217dee17cbc393a5700eb1'
  checksumType  = 'sha1' #default is md5, can also be sha1
}

Install-ChocolateyPackage @packageArgs
