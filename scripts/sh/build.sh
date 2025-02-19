DELETE_BUILD_DIR=false
IGNORE_TESTS="ON"

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -c|--clean)
            DELETE_BUILD_DIR=true
            ;;
        -t|--tests)
            IGNORE_TESTS="OFF"
            ;;
        *)
            echo "Usage: $0 [-c|--clean] [-t|--tests]"
            echo "  -c, --clean  Delete the build directory before building"
            echo "  -t, --tests  Include tests in the build"
            exit 1
            ;;
    esac
    shift
done

if $DELETE_BUILD_DIR; then
    rm -rf build
fi

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DIGNORE_TESTS=${IGNORE_TESTS} -G Ninja ..
cmake --build .
