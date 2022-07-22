#include "map_renderer.h"

using namespace domain;
using namespace renderer;
using namespace svg;
using namespace std;
using namespace geo;

namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

MapRenderer::MapRenderer(const renderer::RenderSettings& render_settings)
    : render_settings_(render_settings) {
}

MapRenderer::MapRenderer(const serialization::RenderSettings& render_settings) {
    render_settings_.width = render_settings.width();
    render_settings_.height = render_settings.height();
    render_settings_.padding = render_settings.padding();
    render_settings_.line_width = render_settings.line_width();
    render_settings_.stop_radius = render_settings.stop_radius();
    render_settings_.bus_label_font_size = render_settings.bus_label_font_size();
    render_settings_.stop_label_font_size = render_settings.stop_label_font_size();
    render_settings_.underlayer_width = render_settings.underlayer_width();
    
    render_settings_.bus_label_offset = {render_settings.bus_label_offset().x(), render_settings.bus_label_offset().y()};
    render_settings_.stop_label_offset = {render_settings.stop_label_offset().x(), render_settings.stop_label_offset().y()};
    
    render_settings_.underlayer_color = ReadSerializedColor(render_settings.underlayer_color());
    
    for (size_t i = 0; i < render_settings.color_palette_size(); ++i) {
        render_settings_.color_palette.push_back(ReadSerializedColor(render_settings.color_palette(i)));
    }
}
                                                 
svg::Color MapRenderer::ReadSerializedColor(const serialization::Color& color) {
    svg::Color result;
    if (color.color_string().size() > 0) {
        return Color(color.color_string());
    }
    int red, green, blue;
    red = color.color_rgb(0);
    green = color.color_rgb(1);
    blue = color.color_rgb(2);
    if (color.is_rgb()) {
        return Rgb(red, green, blue);
    }
    double opasity = color.opasity();
    return Rgba(red, green, blue, opasity);
}
                                                 
svg::Document MapRenderer::Render(const vector<BusPtr>& buses, const vector<StopPtr>& stops) const {
    
    vector<Coordinates> all_stop_coords;
    CalcAllStopsCoordinates(buses, all_stop_coords);
    SphereProjector projector(all_stop_coords.begin(), all_stop_coords.end(), render_settings_.width,
                              render_settings_.height, render_settings_.padding);
    
    svg::Document doc;
    RenderRoutes(buses, projector, doc);
    RenderRouteNames(buses, projector, doc);
    RenderStops(stops, projector, doc);
    RenderStopNames(stops, projector, doc);
    return doc;
}
                                                 
                                                 


void MapRenderer::RenderRoutes(const vector<BusPtr>& buses, const SphereProjector& projector, svg::Document& doc) const {
    
    size_t palette_colors_count = render_settings_.color_palette.size();
    size_t color_counter = 0;
    for (const auto& bus : buses) {
        Polyline route;
        route.SetFillColor(NoneColor);
        route.SetStrokeWidth(render_settings_.line_width);
        route.SetStrokeLineCap(StrokeLineCap::ROUND);
        route.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        route.SetStrokeColor(render_settings_.color_palette[color_counter]);
        if (!(bus->stops).empty()) {
            color_counter = (color_counter + 1) % palette_colors_count;
        }
        RenderOneRoute(bus->stops, projector, route, bus->is_roundtrip);
        doc.Add(route);
    }
}

void MapRenderer::RenderOneRoute(const vector<StopPtr>& stops, const SphereProjector& projector,
                                 Polyline& route, bool is_roundtrip) const {
    
    vector<Point> points;
    points.reserve(stops.size());
    for (const auto& stop : stops) {
        Point point = projector(stop->coordinates);
        route.AddPoint(point);
        points.push_back(point);
    }
    if (is_roundtrip) {
        return;
    }
    
    points.pop_back();
    reverse(points.begin(), points.end());
    for (const Point& point : points) {
        route.AddPoint(point);
    }
}

void MapRenderer::RenderRouteNames(const vector<BusPtr>& buses, const SphereProjector& projector, svg::Document& doc) const {
    
    size_t palette_colors_count = render_settings_.color_palette.size();
    size_t color_counter = 0;
    
    for (const BusPtr& bus : buses) {
        if (bus->stops.empty()) {
            continue;
        }
        Text substrate;
        substrate.SetPosition(projector(bus->stops[0]->coordinates));
        substrate.SetOffset(render_settings_.bus_label_offset);
        substrate.SetFontSize((int) render_settings_.bus_label_font_size);
        const string font_family = "Verdana";
        substrate.SetFontFamily(font_family);
        const string font_weight = "bold";
        substrate.SetFontWeight(font_weight);
        substrate.SetData(bus->name);
        
        Text text = substrate;
        
        substrate.SetFillColor(render_settings_.underlayer_color);
        substrate.SetStrokeColor(render_settings_.underlayer_color);
        substrate.SetStrokeWidth(render_settings_.underlayer_width);
        substrate.SetStrokeLineCap(StrokeLineCap::ROUND);
        substrate.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        
        doc.Add(substrate);
        
        text.SetFillColor(render_settings_.color_palette[color_counter]);
        color_counter = (color_counter + 1) % palette_colors_count;
        
        doc.Add(text);
        
        if (!bus->is_roundtrip && bus->stops[0] != bus->stops.back()) {
            substrate.SetPosition(projector(bus->stops.back()->coordinates));
            text.SetPosition(projector(bus->stops.back()->coordinates));
            doc.Add(substrate);
            doc.Add(text);
        }
    }
}

void MapRenderer::RenderStops(const vector<StopPtr>& stops, const SphereProjector& projector, svg::Document& doc) const {
    
    for (const StopPtr& stop : stops) {
        Circle circle;
        circle.SetCenter(projector(stop->coordinates));
        circle.SetRadius(render_settings_.stop_radius);
        const Color color = "white";
        circle.SetFillColor(color);
        doc.Add(circle);
    }
}

void MapRenderer::RenderStopNames(const vector<StopPtr>& stops, const SphereProjector& projector, svg::Document& doc) const {
    
    for (const StopPtr& stop : stops) {
        Text substrate;
        substrate.SetPosition(projector(stop->coordinates));
        substrate.SetOffset(render_settings_.stop_label_offset);
        substrate.SetFontSize(render_settings_.stop_label_font_size);
        const string font_family = "Verdana";
        substrate.SetFontFamily(font_family);
        substrate.SetData(stop->name);
        
        Text text = substrate;
        
        substrate.SetFillColor(render_settings_.underlayer_color);
        substrate.SetStrokeColor(render_settings_.underlayer_color);
        substrate.SetStrokeWidth(render_settings_.underlayer_width);
        substrate.SetStrokeLineCap(StrokeLineCap::ROUND);
        substrate.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        
        doc.Add(substrate);
        
        const Color color = "black";
        
        text.SetFillColor(color);
                          
        doc.Add(text);
        
    }
}

void MapRenderer::SetRenderSettings(const RenderSettings& render_settings) {
    render_settings_ = render_settings;
}

void MapRenderer::CalcAllStopsCoordinates(const vector<BusPtr>& buses, vector<Coordinates>& all_stops_coords) const {
    
    for (const auto& bus : buses) {
        for (const auto& stop : bus->stops) {
            all_stops_coords.push_back(stop->coordinates);
        }
    }
}

} // namespace renderer

