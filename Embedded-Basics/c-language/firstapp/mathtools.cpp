#include <stdexcept>
double add(double a, double b) {
    return a + b;
}
double multiply(double a, double b) {
    return a * b;
}
double subtract(double a, double b) {
    return a - b;
}
double divide(double a, double b) {
    if (b != 0) {
        return a / b;
    } else {
        // Handle division by zero
        //throw std::invalid_argument("Division by zero");
        return 0; // Return 0 for division by zero
    }
}
