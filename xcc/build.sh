if [ "$1" == "xcc" ]; then
    g++ -std=c++23 -o xcc main.cpp
else
    echo "Unknown target platform: $1"
fi
