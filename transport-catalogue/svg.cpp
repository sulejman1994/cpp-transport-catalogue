#include "svg.h"

#include <map>
#include <utility>

using namespace std;

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

ostream& operator << (ostream& out, const StrokeLineCap& stroke_linecap) {
    out << strokelinecap_to_string.at(stroke_linecap);
    return out;
}

ostream& operator << (ostream& out, const StrokeLineJoin& stroke_linejoin) {
    out << strokelinejoin_to_string.at(stroke_linejoin);
    return out;
}

ostream& operator << (ostream& out, Color color) {
    visit(Visitor{out}, color);
    return out;
}


// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}



// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const  {
    auto& out = context.out;
    out << "<polyline points=\"";
    size_t counter = 0;
    size_t n = points_.size();
    for (const Point& point : points_) {
        ++counter;
        if (counter != n) {
        out << point.x << "," << point.y << " ";
        } else {
            out << point.x << "," << point.y;
        }
    }
    out << "\" ";
    RenderAttrs(out);
    out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const  {
    auto& out = context.out;
    out << "<text ";
    RenderAttrs(out);
    out << "x=" << "\"" << pos_.x << "\" " << "y=" << "\"" << pos_.y << "\" ";
    out << "dx=" << "\"" << offset_.x << "\" " << "dy=" << "\"" << offset_.y << "\" ";
    out << "font-size=" << "\"" << font_size_ << "\" ";
    if (!font_family_.empty()) {
        out << "font-family=" << "\"" << font_family_ << "\" ";
    }
    if (!font_weight_.empty()) {
        out << "font-weight=" << "\"" << font_weight_ << "\">";
    } else {
        out << ">";
    }
    
    const map<char, string> conversions = {{'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}, {'&', "&amp;"}};
    
    for (char c : data_) {
        if (conversions.count(c) == 0) {
            out << c;
        } else {
            out << conversions.at(c);
        }
    }
    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << endl;
    for (const auto& obj : objects_) {
        obj->Render(out);
    }
    out << "</svg>" << endl;
}

}  // namespace svg
