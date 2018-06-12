
function libs() {
libtool -static -o libxnrtc_arm64.a ./libs/*.a
libtool -static -o libxnrtc_x64.a ./libsx64/*.a
}

$@
