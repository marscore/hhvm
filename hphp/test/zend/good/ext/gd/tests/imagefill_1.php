<?php <<__EntryPoint>> function main() {
$im = imagecreate(100,100);
$white = imagecolorallocate($im, 255,255,255);
$blue = imagecolorallocate($im, 0,0,255);
$green = imagecolorallocate($im, 0,255,0);

print_r(imagecolorat($im, 0,0));
imagefill($im, 0,0,$white + 3);
print_r(imagecolorat($im, 0,0));
imagedestroy($im);
}
