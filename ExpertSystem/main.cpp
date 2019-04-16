#include <iostream>
#include <list>
#include <map>
#include <string>
#include <stack>
#include <fstream>
#include <algorithm>
#include <regex>

#define NORMAL  "\x1B[0m"
#define FALSE  "\x1B[31mfalse"
#define TRUE  "\x1B[34mtrue"

#define COLOR(result) result ? TRUE : FALSE

/*
 * Make separate class
 * Add data validation
 * Incapsulate data
 * Make all private, pass filename and parameter to instance
 * Add one public method Solve(const char query)
 */
struct Helper {
    static bool isOp(const char c) {
        return c == '+' || c == '|' || c == '^';
    }

    static bool isAlpha(const char c) {
        return c >= 65 && c <= 90;
    }

    static bool evaluateOP(bool lhs, char c, bool rhs) {
        switch (c) {
            case '+':
                return lhs && rhs;
            case '|':
                return lhs || rhs;
            case '^':
                return lhs ^ rhs;
            default:
                return false;
        }
    }

    static int getPriority(char op) {
        switch (op) {
            case '!':
                return 4;
            case '+':
                return 3;
            case '|':
                return 2;
            case '^':
                return 1;
            default:
                return 0;
        }
    }

    static bool compare(char lhs, char rhs) {
        return getPriority(lhs) > getPriority(rhs);
    }

    static bool isPairParenthesis(const std::string &expr) {
        auto count = 0;
        for (const auto c: expr) {
            if (c == '(') ++count;
            if (c == ')') --count;
        }
        return count == 0;
    }
};

struct Rule
{
    std::string lhs;
    std::string rhs;
    std::string lhsP;

    Rule(std::string &_lhs, std::string &_rhs) : lhs(_lhs), rhs(_rhs) {
        lhsP = transform(lhs);
    }

    std::string transform(const std::string &expr) {
        std::stack<char> _stack;
        std::string newExpr;
        for (const auto &c : expr) {
            if (Helper::isAlpha(c)) {
                newExpr += c;
            } else if (c == '(') {
                _stack.push(c);
            } else if (c == ')') {

                while (_stack.top() != '(') {
                    newExpr += _stack.top();
                    _stack.pop();
                }
                if (_stack.empty() || _stack.top() != '(') {
                    printf("Error in expr!\n");
                } else {
                    _stack.pop();
                }

            } else if (Helper::isOp(c)) {
                    while (!_stack.empty() && (Helper::compare(_stack.top(), c)  || c == '!')) {
                        newExpr += _stack.top();
                        _stack.pop();
                    }
                    _stack.push(c);
            } else if (c == '!') {
                _stack.push(c);
            }
        }
        while (!_stack.empty()) {
            newExpr += _stack.top();
            _stack.pop();
        }
        return newExpr;
    }
};

struct ExpertSystem {
    std::map<char, bool> facts;
    std::list< Rule > rules;
    std::string queries;
    std::string goals;

    bool isInMap(const char c) {
        for (const auto pair : facts) {
            if (pair.first == c) return true;
        }
        return false;
    }

    bool evaluateExpr(const std::string &expr) {
        std::stack<bool> _stack;
        if (expr.empty()) {
            return false;
        }
        for (const char c : expr) {
            if (Helper::isAlpha(c)) {
                if (!isInMap(c) && goals.find(c) == std::string::npos) {
                    goals += c;
                    std::string negativeStr = "!";
                    negativeStr += c;
                    std::list<bool> resultList;
                    bool positive;
                    for (auto &rule : rules) {
                        if (rule.rhs.find(c) != std::string::npos) {
                            positive = true;

                            if (rule.rhs.find(negativeStr) != std::string::npos) {
                                positive = false;
                            }
                            resultList.push_back(evaluateExpr(rule.lhsP) == positive);
                        }
                    }
                    facts.insert(std::pair<char, bool>(c, std::any_of(resultList.begin(), resultList.end(), [](bool elem) {
                        return elem;
                    })));
                }
                _stack.push(facts[c]);
            } else if (Helper::isOp(c)) {
                auto lhs = _stack.top();
                _stack.pop();
                auto rhs = _stack.top();
                _stack.pop();
                _stack.push(Helper::evaluateOP(lhs, c, rhs));
            } else if (c == '!') {
                auto res = !_stack.top();
                _stack.pop();
                _stack.push(res);
            }
        }
        return _stack.top();
    }
};

int main(int argc, char **argv) {

    auto *system = new ExpertSystem();

    if (!system) {
        printf("Cannot create object(\n");
        exit(1);
    }

    std::ifstream file;
    std::string line;
    std::regex factRegex("=[A-Z]{0,}");
    std::regex queryRegex("\\?[A-Z]{1,}");
    std::regex ruleRegex("^(([!]?[(]?[!]?[A-Z]{1,1}[)]?[+|^]?)|([!]?[(]?[!]?[A-Z]{1,1}[+|^]{1,1}[!]?[A-Z]{1,1}[)]?))+");
    std::regex ruleRegexWrongEnd("[+^|]$");
    std::regex ruleRegexAdditional("^[A-Z]{2,}");
    if (argc != 2) {
        printf("Please, provide file\n");
        exit(1);
    }
    file.open(argv[1]);
    if (!file) {
        printf("Can't open file\n");
        exit(1);
    }

    bool queryPresented = false;
    bool factsPresented = false;
    while (std::getline(file, line)) {
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        if (line.empty() || line[0] == '#') continue;
        std::string rule = line.substr(0, line.find('#'));

        if (line[0] == '?') {
            if (!std::regex_match(rule, queryRegex)) {
                printf("Invalid query syntax!\n");
                exit(1);
            }
            system->queries = rule.substr(1, rule.size());
            queryPresented = true;
        }

        else if (rule[0] == '=') {
            if (!std::regex_match(rule, factRegex)) {
                printf("Invalid fact syntax!\n");
                exit(1);
            }
            factsPresented= true;
            auto i = 1;
            while (rule[i] && Helper::isAlpha(rule[i])) {
                system->facts.insert(std::pair<char, bool>(rule[i], true));
                ++i;
            }
        }

        else if (!line.empty()) {
            std::string left = rule.substr(0, rule.find("=>"));
            std::string right = rule.substr(rule.find("=>") + 2);
            if (!std::regex_match(left, ruleRegex) || !std::regex_match(right, ruleRegex)
                || std::regex_search(left, ruleRegexAdditional) || std::regex_search(right, ruleRegexAdditional)
                || std::regex_search(left, ruleRegexWrongEnd) || std::regex_search(right, ruleRegexWrongEnd)
                || !Helper::isPairParenthesis(left) || !Helper::isPairParenthesis(right)) {
                printf("Invalid rule syntax\n");
                exit(1);
            }
            system->rules.emplace_back(Rule(left, right));
        }
    }

    if (!factsPresented || !queryPresented || system->rules.empty()) {
        printf("Error in file\n");
        exit(1);
    }

    for (char query : system->queries) {
        std::cout << "started evaluating for query: " << query << std::endl;
        bool positive;
        if (!system->isInMap(query)) {

            std::string negativeStr = "!";
            negativeStr += query;
            std::list<bool> resultList;
            for (auto &rule : system->rules) {
                if (rule.rhs.find(query) != std::string::npos) {
                    positive = true;

                    if (rule.rhs.find(negativeStr) != std::string::npos) {
                        positive = false;
                    }
                    resultList.push_back(system->evaluateExpr(rule.lhsP) == positive);
                }
            }
            system->facts.insert(std::pair<char, bool>(query, std::any_of(resultList.begin(), resultList.end(), [](bool elem) {
                return elem;
            })));

        }
        printf("%c: %s%s\n", query, COLOR(system->facts[query]), NORMAL);
    }
    return 0;
}
