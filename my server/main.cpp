#include "httplib.h"
#include <iostream>
#include <string>

int main() {
    httplib::Server svr;

    // 1. Send the HTML Form when you open the main page
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string html = R"(
            <html>
            <body style="font-family: sans-serif; padding: 40px;">
                <h2>My C++ Web Calculator</h2>
                <form action="/add" method="get">
                    <input type="number" step="any" name="a" placeholder="First number" required> +
                    <input type="number" step="any" name="b" placeholder="Second number" required>
                    <button type="submit">Calculate</button>
                </form>
            </body>
            </html>
        )";
        res.set_content(html, "text/html");
    });

    // 2. Do the C++ math when the form button is clicked
    svr.Get("/add", [](const httplib::Request& req, httplib::Response& res) {
        double a = std::stod(req.get_param_value("a"));
        double b = std::stod(req.get_param_value("b"));
        double sum = a + b;

        std::string reply = "<h2>Answer from C++: " + std::to_string(sum) + "</h2>"
                            "<a href='/'>Go Back</a>";
        res.set_content(reply, "text/html");
    });

    std::cout << "Server running! Open http://localhost:8080 in your browser.\n";
    svr.listen("0.0.0.0", 8080); // Starts listening on port 8080
    return 0;
}
