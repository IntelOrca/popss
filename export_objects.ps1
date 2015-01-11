$popssPath = ".\build\Debug_Win32\popss.exe";
$inputDir = ".\export\objects\"
$outputDir = ".\data\objects\"
foreach ($item in Get-ChildItem $inputDir) {
    $expr = $popssPath + " convobj " + $item.FullName + " " + ($outputDir + $item.BaseName + ".object");
    Invoke-Expression $expr;
}