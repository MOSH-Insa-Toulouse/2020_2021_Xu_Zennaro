function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var len = bytes.length;
  //decoding
  var decoded = 0;
  //minus 0x30 for ascii -> dec
  if (len == 2){
    decoded = bytes[0]*10 + bytes[1] - 528;
  }else if (len == 1){
    decoded = bytes[0] - 48;
  }else if (len == 3){
    decoded = bytes[0]*100 + bytes[1]*10 + bytes[2] - 5328;
  }
  decoded /= 100;
  s = String(decoded)+"V"
  return {voltage:s, value:decoded};
}