<?hh <<__EntryPoint>> function main() {
echo sprintf("%e\n", 1.123456789);
echo sprintf("%.10e\n", 1.123456789);
echo sprintf("%.0e\n", 1.123456789);
echo sprintf("%.1e\n", 1.123456789);
echo sprintf("%5.1e\n", 1.123456789);
}
