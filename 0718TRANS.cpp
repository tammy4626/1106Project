#include <algorithm>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#define CONST_BTN_NAMED_OVERLAP 0.6 // 1 requires two area be exactly same, 0 means that two area will be judged as same if there are any overlap
#define CONST_ELEMENT_BELONG_OVERLAP 0.5 // the ratio of (overlap area / element area) to judge if elements in this container or row

struct Element { // A type that store each element
    string type;
    double x1, x2, y1, y2;
    vector<Element> children; // Stores all elements directly belong it
    Element() = default;
    Element(string type, const Element &ref, int no) : type(type) { // Use parent row data generate container position
        x1 = ref.x1, x2 = ref.x2;
        double width = (ref.y2 - ref.y1) / map<string, int>({{"single", 1}, {"double", 2}, {"quadruple", 4}})[type];
        y1 = ref.y1 + no * width, y2 = ref.y1 + (no + 1) * width;
    }
    void rows_beautify() { // Auto complete row's containers
        assert(type == "row");
        for (int i = 1; i < (int)children.size(); ++i) if (children[i].type != children[i - 1].type) return;
        if (children.empty() || children[0].type == "single") {
            if (x1 > 0.1) {
                children.clear();
                children.push_back(Element("single", *this, 0));
            }
        } else if (children[0].type == "double") {
            children.clear();
            children.push_back(Element("double", *this, 0));
            children.push_back(Element("double", *this, 1));
        } else if (children[0].type == "quadruple") {
            children.clear();
            children.push_back(Element("quadruple", *this, 0));
            children.push_back(Element("quadruple", *this, 1));
            children.push_back(Element("quadruple", *this, 2));
            children.push_back(Element("quadruple", *this, 3));
        }
    }

    bool operator < (const Element &b) const {
        return make_pair(x1, y1) < make_pair(b.x1, b.y1);
    }
};

bool is_container(string s) {
    return s == "single" || s == "double" || s == "quadruple";
}
bool is_leaf(string s) {
    return s == "text" || s == "btn-active" || s == "small-title" ||
        s == "start" || s == "menu" || s == "next" || s == "back" || s == "play" || s == "cancel";
}
bool is_btn_name(string s) {
    return s == "start" || s == "menu" || s == "next" || s == "back" || s == "play" || s == "cancel";
}

double calc_area(const Element &a) {
    return (a.y2 - a.y1) * (a.x2 - a.x1);
}
double calc_overlap(const Element &a, const Element &b) { // Return overlap area of two element
    double x = min(a.x2, b.x2) - max(a.x1, b.x1);
    double y = min(a.y2, b.y2) - max(a.y1, b.y1);
    if (x <= 0 || y <= 0) return 0;
    return x * y;
}

vector<Element> input(char *filename) { // process input
    auto get_string_type = [&] (const int type) -> string { // get string type by numerical type
        if (type == 0) return "None";
        if (type == 1) return "text";
        if (type == 2) return "btn-active";
        if (type == 3) return "row";
        if (type == 4) return "single";
        if (type == 5) return "double";
        if (type == 6) return "quadruple";
        if (type == 7) return "small-title";
        if (type == 8) return "start";
        if (type == 9) return "menu";
        if (type == 10) return "next";
        if (type == 11) return "back";
        if (type == 12) return "play";
        if (type == 13) return "cancel";
        return "ERROR";
    };
    ifstream i(filename); // Open file (C++ style)
    vector<Element> v; Element e; int t; double w, h;
    while (i >> t) {
        e.type = get_string_type(t);
        i >> e.x1 >> e.y1 >> w >> h;
        e.x1 -= w / 2;
        e.y1 -= h / 2;
        e.x2 = e.x1 + w;
        e.y2 = e.y1 + h;
        swap(e.x1, e.y1);
        swap(e.x2, e.y2); // x is axis from up to down (left to right in yolo's output)
        v.push_back(e);
        cerr << "Add element " << e.type << " at " << e.x1 << " " << e.y1 << " " << e.x2 << " " << e.y2 << endl;
    }
    return v;
}

vector<Element> name_buttons(vector<Element> elements) {
    vector<bool> skip(elements.size(), false);
    for (int i = 0; i < (int)elements.size(); ++i) if (!skip[i] && is_btn_name(elements[i].type)) {
        int name_owner = -1;
        double best_overlap_ratio = 0;
        for (int j = 0; j < (int)elements.size(); ++j) if (i != j && !skip[j] && elements[i].type == "btn-active") {
            double overlap_area = calc_overlap(elements[i], elements[j]);
            double total_area = calc_area(elements[i]) + calc_area(elements[j]);
            if (overlap_area * 2 > total_area * CONST_BTN_NAMED_OVERLAP) {
                double overlap_ratio = overlap_area * 2 / total_area;
                if (overlap_ratio > best_overlap_ratio) {
                    name_owner = j;
                    best_overlap_ratio = overlap_ratio;
                }
            }
        }
        if (name_owner != -1) {
            cerr << "btn name " << elements[i].type << " at " << elements[i].x1 << " " << elements[i].y1 << " is name of btn at " << elements[name_owner].x1 << " " << elements[name_owner].y1 << " with overlap ratio " << best_overlap_ratio << endl;
            skip[i] = skip[name_owner] = true;
            elements[name_owner].type = elements[i].type;
        }
    }
    vector<Element> new_elements;
    for (int i = 0; i < (int)elements.size(); ++i) if (!skip[i]) new_elements.push_back(elements[i]);
    return new_elements;
}

ofstream get_output_name(char *c) {
    string s(c, c + strlen(c));
    s = s.substr(0, s.size() - 3);
    s += "trans_txt";
    return ofstream(s);
}
void output(ofstream &cout, const vector<Element> &rows) {
    cout << "<START>" << endl;
    bool header = true;
    for (auto &r : rows) { // Output to stdout
        if (header) {
            header = false;
            cout << "header {" << endl;
            cout << "\t";
            for (size_t i = 0; i < r.children.size(); ++i) {
                if (i > 0) cout << ", ";
                cout << r.children[i].type;
            }
            cout << endl;
            cout << "}" << endl;
        } else {
            cout << "row {" << endl;
            for (auto &c : r.children) {
                cout << "\t" << c.type << " {" << endl;
                cout << "\t\t";
                for (size_t i = 0; i < c.children.size(); ++i) {
                    if (i > 0) cout << ", ";
                    cout << c.children[i].type;
                }
                cout << endl;
                cout << "\t}" << endl;
            }
            cout << "}" << endl;
        }
    }
    cout << "<END>" << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " input" << endl; // Add input filename after this excutable file in command line
        exit(1);
    }
    auto elements = input(argv[1]); 
    auto cout = get_output_name(argv[1]); // override cout with file output
    cerr << fixed << setprecision(3); // set error output format
    elements = name_buttons(elements); // process about `named` buttons
    sort(elements.begin(), elements.end()); // sort all elements from up to down, from left to right

    vector<Element> rows;
    for (auto e : elements) if (e.type == "row") rows.push_back(e);

    for (auto e : elements) if (is_container(e.type)) {
        Element *best_row = nullptr;
        double cur_overlap_area = 0;
        for (auto &r : rows) { // For each container, find the row overlaps most
            double overlap_area = calc_overlap(e, r);
            if (overlap_area > calc_area(e) * CONST_ELEMENT_BELONG_OVERLAP && overlap_area > cur_overlap_area) {
                best_row = &r;
                cur_overlap_area = overlap_area;
            }
        }
        if (best_row == nullptr) {
            cerr << "Container " << e.type << " at " << e.x1 << " " << e.y1 << " doesn't belong to any row." << endl;
        } else {
            cerr << "Container " << e.type << " at " << e.x1 << " " << e.y1 << " belongs to row " << best_row->x1 << " " << best_row->y1 << " with overlap area: " << cur_overlap_area << endl;
            best_row->children.push_back(e);
        }
    }

    for (int i = 1; i < (int)rows.size(); ++i) rows[i].rows_beautify();
    sort(rows.begin(), rows.end());

    for (auto e : elements) if (is_leaf(e.type)) {
        Element *best_container = nullptr;
        double cur_overlap_area = 0;
        for (auto &r : rows) {
            for (auto &c : r.children) if (is_container(c.type)) { // For each text/btn, find the container overlaps most
                double overlap_area = calc_overlap(e, c);
                if (overlap_area > calc_area(e) * CONST_ELEMENT_BELONG_OVERLAP && overlap_area > cur_overlap_area) {
                    best_container = &c;
                    cur_overlap_area = overlap_area;
                }
            }
        }

        if (best_container == nullptr) {
            cerr << "Object " << e.type << " at " << e.x1 << " " << e.y1 << " doesn't belong to any container." << endl;
            Element *best_row = nullptr;
            double cur_overlap_area = 0;
            for (auto &r : rows) {
                double overlap_area = calc_overlap(e, r);
                if (overlap_area > calc_area(e) * CONST_ELEMENT_BELONG_OVERLAP && overlap_area > cur_overlap_area) {
                    best_row = &r;
                    cur_overlap_area = overlap_area;
                }
            }
            if (best_row == nullptr) {
                cerr << "Object " << e.type << " at " << e.x1 << " " << e.y1 << " doesn't belong to any row." << endl;
            } else {
                cerr << "Object " << e.type << " at " << e.x1 << " " << e.y1 << " belongs to row " << best_row->x1 << " " << best_row->y1 << " with overlap area: " << cur_overlap_area << endl;
                best_row->children.push_back(e);
            }
        } else {
            cerr << "Object " << e.type << " at " << e.x1 << " " << e.y1 << " belongs to container " << best_container->type << " at " << best_container->x1 << " " << best_container->y1 << " with overlap area: " << cur_overlap_area << endl;
            best_container->children.push_back(e);
        }
    }

    output(cout, rows);
}
