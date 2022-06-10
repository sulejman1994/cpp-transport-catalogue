#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <map>
#include <variant>

using std::vector, std::string, std::unique_ptr, std::make_unique, std::move, std::optional, std::map, std::ostream, std::variant, std::monostate, std::visit;

namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

    
class ObjectContainer {
public:
    virtual void AddPtr(unique_ptr<Object>&& obj) = 0;
    
    template <typename Obj>
    void Add(Obj obj);
    
    virtual ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    
    virtual ~Drawable() = default;
};



struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {

    }
    unsigned short red = 0;
    unsigned short green = 0;
    unsigned short blue = 0;
};

struct Rgba : Rgb {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : Rgb(r, g, b), opacity(o) {
        
    }
    double opacity = 1.0;
};



using Color = variant<monostate, string, Rgb, Rgba>;

inline const string NoneColor("none");


struct Visitor {
    ostream& out;
    
    void operator() (monostate) const {
        out << NoneColor;
    }
    void operator() (string color) const {
        out << color;
    }
    void operator() (Rgb rgb) const {
        out << "rgb(" << rgb.red << "," << rgb.green << "," << rgb.blue << ")";
    }
    void operator() (Rgba rgba) const {
        out << "rgba(" << rgba.red << "," << rgba.green << "," << rgba.blue << "," << rgba.opacity << ")";
    }
};

ostream& operator << (ostream& out, Color color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

const map<StrokeLineCap, string> strokelinecap_to_string = {{StrokeLineCap::BUTT, "butt"}, {StrokeLineCap::ROUND, "round"}, {StrokeLineCap::SQUARE, "square"}};

ostream& operator << (ostream& out, const StrokeLineCap& stroke_linecap);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

const map<StrokeLineJoin, string> strokelinejoin_to_string = {{StrokeLineJoin::ARCS, "arcs"}, {StrokeLineJoin::BEVEL, "bevel"},{StrokeLineJoin::MITER, "miter"}, {StrokeLineJoin::MITER_CLIP, "miter-clip"}, {StrokeLineJoin::ROUND, "round"}};

ostream& operator << (ostream& out, const StrokeLineJoin& stroke_linejoin);

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = move(color);
        return AsOwner();
    }
    
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = move(line_cap);
        return AsOwner();
    }
    
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = move(line_join);
        return AsOwner();
    }
    
    virtual ~PathProps() = default;

protected:
    void RenderAttrs(std::ostream& out) const;
    
private:
    optional<Color> fill_color_;
    optional<Color> stroke_color_;
    optional<double> stroke_width_;
    optional<StrokeLineCap> stroke_linecap_;
    optional<StrokeLineJoin> stroke_linejoin_;
    
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }
};
    
template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {

    if (fill_color_) {
        out << "fill=\"" << *fill_color_ << "\" ";
    }
    if (stroke_color_) {
        out << "stroke=\"" << *stroke_color_ << "\" ";
    }
    if (stroke_width_) {
        out << "stroke-width=\"" << *stroke_width_ << "\" ";
    }
    if (stroke_linecap_) {
        out << "stroke-linecap=\"" << *stroke_linecap_ << "\" ";
    }
    if (stroke_linejoin_) {
        out << "stroke-linejoin=\"" << *stroke_linejoin_ << "\" ";
    }
}
    
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    
    vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);
    
private:
    void RenderObject(const RenderContext& context) const override ;

    Point pos_;
    Point offset_;
    size_t font_size_ = 1;
    string font_weight_;
    string font_family_;
    string data_;
};

class Document : public ObjectContainer {
public:
    
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

private:
    vector<unique_ptr<Object>> objects_;
};
    
template <typename Obj>
void ObjectContainer::Add(Obj obj) {
    auto ptr = make_unique<Obj> (move(obj));
    AddPtr(move(ptr));
}

}  // namespace svg



