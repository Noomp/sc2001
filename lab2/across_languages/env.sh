# Shared environment and build for the cross-language experiment.
L=~/sc2001-langs
export PATH=$L/node/bin:$L/go/bin:$L/jdk/bin:$L/dotnet:$L/cargo/bin:$PATH
export CARGO_HOME=$L/cargo RUSTUP_HOME=$L/rustup GOCACHE=$L/gocache GOPATH=$L/gopath
export DOTNET_CLI_TELEMETRY_OPTOUT=1 DOTNET_NOLOGO=1 DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1 DOTNET_CLI_HOME=$L/dotnethome

LANGS=(C C++ Rust Go Java C# JavaScript Python)

build_all() {
    gcc -O2 -o gen2 gen2.c -lm &&
    gcc -O2 -o x_c x_c.c -lm &&
    g++ -O2 -std=c++17 -o x_cpp x_cpp.cpp &&
    rustc --edition 2021 -C opt-level=3 -o x_rs x_rs.rs &&
    go build -o x_go x_go.go &&
    javac XBench.java &&
    dotnet build xcs -c Release -o xcs/out > /dev/null
}

cmd_for() {
    case $1 in
        C) echo "./x_c" ;;
        C++) echo "./x_cpp" ;;
        Rust) echo "./x_rs" ;;
        Go) echo "./x_go" ;;
        Java) echo "java -Xms1g -Xmx24g -cp . XBench" ;;
        C#) echo "dotnet xcs/out/XBench.dll" ;;
        JavaScript) echo "node --max-old-space-size=8192 x_js.js" ;;
        Python) echo "python3 x_py.py" ;;
    esac
}
