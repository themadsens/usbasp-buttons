while true ;do
    usbtool -v 0x16c0 -p 0x05dc -V "madsensoft.dk" -P "usbasp-buttons" -t 15000 -e 1 interrupt in 
    usbtool -v 0x16c0 -p 0x05dc -V "madsensoft.dk" -P "usbasp-buttons" -e 0 -b control in vendor endpoint 3 0 0 \
        || sleep 10
done \
    | grep -w BTN

