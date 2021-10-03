#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <cairo.h>
#include <cairo-svg.h>
#include <json-c/json.h>

#include "chart-generator.h"
#include "config.h"

void explicit_bzero(void *s, size_t n);

char output_path[2048] = ".";

cairo_status_t write_image_file(void *closure, const unsigned char *data, const unsigned int length)
{
    return (fwrite(data, 1, length, (FILE *)closure) == length) ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
}

void generate_chart(const char *buffer)
{
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(buffer);

    struct json_object *empty;
    json_object_object_get_ex(parsed_json, "empty", &empty);

    if (!json_object_get_boolean(empty)) {
        struct json_object *symbol;
        json_object_object_get_ex(parsed_json, "symbol", &symbol);
        const char *s_symbol = json_object_get_string(symbol);

        fprintf(stderr, "%s\n", s_symbol);

        struct json_object *candles_array;
        json_object_object_get_ex(parsed_json, "candles", &candles_array);
        size_t candles_array_length = json_object_array_length(candles_array);

        Candle candles[candles_array_length];
        double min_low = 0.0;
        double max_high = 0.0;
        int    max_volume = 0;
        int    min_datetime = INT_MAX;
        int    max_datetime = 0;
        const int unit_period = 24 * 60 * 60; // TODO: should be automatically detected

        for (size_t i = 0; i < candles_array_length; i++) {
            struct json_object *candles_array_object = json_object_array_get_idx(candles_array, i);
            struct json_object *open;
            struct json_object *high;
            struct json_object *low;
            struct json_object *close;
            struct json_object *volume;
            struct json_object *datetime;

            json_object_object_get_ex(candles_array_object, "open",     &open);
            json_object_object_get_ex(candles_array_object, "high",     &high);
            json_object_object_get_ex(candles_array_object, "low",      &low);
            json_object_object_get_ex(candles_array_object, "close",    &close);
            json_object_object_get_ex(candles_array_object, "volume",   &volume);
            json_object_object_get_ex(candles_array_object, "datetime", &datetime);

            double d_open =     json_object_get_double(open);
            double d_high =     json_object_get_double(high);
            double d_low =      json_object_get_double(low);
            double d_close =    json_object_get_double(close);
            int    i_volume =   json_object_get_int64(volume);
            int    i_datetime = json_object_get_int64(datetime) / 1000;

            Candle c;

            c.open = d_open;
            c.high = d_high;
            c.low = d_low;
            c.close = d_close;
            c.volume = i_volume;
            c.datetime = i_datetime;

            candles[i] = c;

            if (d_low < min_low) min_low = d_low;
            if (d_high > max_high) max_high = d_high;
            if (i_volume > max_volume) max_volume = i_volume;
            if (i_datetime < min_datetime) min_datetime = i_datetime;
            if (i_datetime > max_datetime) max_datetime = i_datetime;
        }

        const int total_amount_of_units = (max_datetime - min_datetime) / unit_period;
        const int candlestick_displacement = CHART_IMAGE_CANDLESTICK_WIDTH + CHART_IMAGE_CANDLESTICK_SPACING;
        const int volumebar_displacement = CHART_IMAGE_VOLUMEBAR_WIDTH + CHART_IMAGE_VOLUMEBAR_SPACING;
        const int max_item_width = (candlestick_displacement > volumebar_displacement) ? candlestick_displacement : volumebar_displacement;
        const int additional_spacing = (CHART_IMAGE_CANDLESTICK_SPACING > CHART_IMAGE_VOLUMEBAR_SPACING) ? CHART_IMAGE_CANDLESTICK_SPACING : CHART_IMAGE_VOLUMEBAR_SPACING;
        const int image_width = max_item_width * total_amount_of_units + additional_spacing;
        const int image_height = CHART_IMAGE_HEIGHT; // Image height is always the same for all charts, but width varies
        const int price_chart_height = image_height - CHART_IMAGE_VOLUMEBAR_MAX_HEIGHT;

#if CHART_IMAGE_VERTICAL_PRICE_SCALE > 0
        // Set fixed scale for all charts
        max_high = CHART_IMAGE_VERTICAL_PRICE_SCALE;
#endif

        cairo_surface_t *surface = cairo_svg_surface_create(NULL, image_width, image_height);
        cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_2);
        cairo_t *cr = cairo_create(surface);
        cairo_set_tolerance(cr, 0.00001);
        // Set background color for the image
        cairo_set_source_rgb(cr, 0.03, 0.03, 0.03);
        cairo_paint(cr);

#if CHART_IMAGE_RENDER_TICKER_NAME == true
        // Slap stock ticker symbol onto the chart
        cairo_save(cr);
        {
            cairo_text_extents_t extents;
            cairo_set_source_rgba(cr, 1, 1, 1, 0.1);
            cairo_set_font_size(cr, 200);
            cairo_text_extents(cr, s_symbol, &extents);
            cairo_move_to(cr, 0 + 10, extents.height + 10);
            cairo_show_text(cr, s_symbol);
        }
        cairo_restore(cr);
#endif

        // Draw horizontal line that separates price and volume charts
        cairo_save(cr);
        {
            const double width = 1;
            cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
            cairo_set_line_width(cr, width);
            cairo_move_to(cr, 0, price_chart_height);
            cairo_line_to(cr, image_width, price_chart_height);
            cairo_stroke(cr);
        }
        cairo_restore(cr);

        // Place vertical lines that indicate beginning of a new year
        for (int i = min_datetime; i < max_datetime; i += 86400) {
            time_t t = i;
            const char *format_jan_1 = "%b %d";
            const char *format_year = "%Y";
            struct tm lt;
            char res_jan_1[32];
            char res_year[32];
            localtime_r(&t, &lt);
            strftime(res_jan_1, sizeof(res_jan_1), format_jan_1, &lt);
            strftime(res_year, sizeof(res_year), format_year, &lt);

            if (strcmp("Jan 01", res_jan_1) == 0) {
                // Add vertical line for every January 1st
                cairo_save(cr);
                {
                    const double width = 1;
                    const double pos_y_start = 0;
                    const double pos_y_end = price_chart_height;
                    const double pos_x = (double)(i - min_datetime) / (double)(max_datetime - min_datetime) * (double)image_width;
                    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
                    cairo_set_line_width(cr, width);
                    cairo_move_to(cr, pos_x, pos_y_start);
                    cairo_line_to(cr, pos_x, pos_y_end);
                    cairo_stroke(cr);
                }
                cairo_restore(cr);
                // Add year indicator
                cairo_save(cr);
                {
                    cairo_text_extents_t extents;
                    const double pos_x = (double)(i - min_datetime) / (double)(max_datetime - min_datetime) * (double)image_width;
                    cairo_set_source_rgba(cr, 1, 1, 1, 0.1);
                    cairo_set_font_size(cr, 100);
                    cairo_text_extents(cr, res_year, &extents);
                    cairo_move_to(cr, pos_x + 10, extents.height + 10);
                    cairo_show_text(cr, res_year);
                }
                cairo_restore(cr);
            }
        }

        for (size_t i = 0; i < candles_array_length; i++) {
            const Candle candle = candles[i];
            const bool is_bullish = candle.close > candle.open;
            const bool is_bearish = candle.close < candle.open;
            const int index = (candle.datetime - min_datetime) / unit_period;

            // Draw candlestick wick
            cairo_save(cr);
            {
                const double width = 1;
                const double pos_y_start = price_chart_height - candle.low / max_high * price_chart_height;
                const double pos_y_end = price_chart_height - candle.high / max_high * price_chart_height;
                const double pos_x = index * (CHART_IMAGE_CANDLESTICK_WIDTH + CHART_IMAGE_CANDLESTICK_SPACING) + ((i == 0) ? 0 : CHART_IMAGE_CANDLESTICK_SPACING);
                if (is_bullish) {
                    cairo_set_source_rgba(cr, 0, 1, 0, 0.75); // Green
                } else if (is_bearish) {
                    cairo_set_source_rgba(cr, 1, 0, 0, 0.75); // Red
                } else {
                    cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 0.75); // Gray
                }
                cairo_set_line_width(cr, width);
                cairo_move_to(cr, pos_x, pos_y_start);
                cairo_line_to(cr, pos_x, pos_y_end);
                cairo_stroke(cr);
            }
            cairo_restore(cr);

            // Draw candlestick block
            cairo_save(cr);
            {
                const double width = CHART_IMAGE_CANDLESTICK_WIDTH;
                const double pos_y_start = price_chart_height - candle.close / max_high * price_chart_height;
                double pos_y_end = price_chart_height - candle.open / max_high * price_chart_height;
                const double pos_x = index * (CHART_IMAGE_CANDLESTICK_WIDTH + CHART_IMAGE_CANDLESTICK_SPACING) + ((i == 0) ? 0 : CHART_IMAGE_CANDLESTICK_SPACING);
                if (is_bullish) {
                    cairo_set_source_rgb(cr, 0, 1, 0); // Green
                } else if (is_bearish) {
                    cairo_set_source_rgb(cr, 1, 0, 0); // Red
                } else {
                    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6); // Gray
                }
                cairo_set_line_width(cr, width);
                cairo_move_to(cr, pos_x, pos_y_start);
                // Move one pixel from the start to display a small dash line
                if (pos_y_start == pos_y_end) {
                    pos_y_end += 1;
                }
                cairo_line_to(cr, pos_x, pos_y_end);
                cairo_stroke(cr);
            }
            cairo_restore(cr);

            // Draw volume bar
            cairo_save(cr);
            {
                const double width = CHART_IMAGE_VOLUMEBAR_WIDTH;
                const double height = CHART_IMAGE_VOLUMEBAR_MAX_HEIGHT * (candle.volume / (double)max_volume);
                const double pos_x = index * (CHART_IMAGE_VOLUMEBAR_WIDTH + CHART_IMAGE_VOLUMEBAR_SPACING) + ((i == 0) ? 0 : CHART_IMAGE_VOLUMEBAR_SPACING);
                const double pos_y = image_height - height;
                cairo_rectangle(cr, pos_x, pos_y, width, height);
                cairo_set_source_rgba(cr, 1, 1, 1, 0.3);
                cairo_fill(cr);
            }
            cairo_restore(cr);
        }

        cairo_surface_flush(surface);

        // Write output to file
        {
            char output_file_path[2048];
            explicit_bzero(output_file_path, sizeof(output_file_path));
            strcpy(output_file_path, output_path);
            strcat(output_file_path, "/");
            strcat(output_file_path, s_symbol);
            strcat(output_file_path, ".png");
            FILE *fp = fopen(output_file_path, "w");

            if (fp != NULL) {
                cairo_surface_write_to_png_stream(surface, write_image_file, fp);
                fclose(fp);
            } else {
                fprintf(stderr, "Error: unable to write to file %s\n", output_file_path);
            }
        }

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }
}

int main(const int argc, const char **argv)
{
    char *buffer = NULL;
    FILE *fp;
    int opt;
    int option_index = 0;

    static const struct option long_options[] = {
        { "output-dir", required_argument, NULL, 'o'   },
        { "help",       no_argument,       NULL, 'h'   },
        { NULL,         0,                 NULL, false }
    };

    while (true) {
        opt = getopt_long(argc, (char**)argv, "o:h:", long_options, &option_index);

        if (opt == -1) {
            break;
        }

        switch (opt) {
            case 0:
                break;

            case 'o':
                strcpy(output_path, optarg);
                break;

            case 'h':
                printf("Usage: %s [OPTIONS] [TARGET] ...\n", argv[0]);
                puts("Options:");
                printf("  -o PATH, --output-dir=PATH       Specify output directory.\n");
                printf("  -h, --help                       Print this help and exit.\n");
                puts("");
                exit(EXIT_SUCCESS);

            default:
                fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        // Ensure the output directory exists
        mkdir(output_path, S_IRWXU);

        // Loop through non-flag/option arguments
        while (optind < argc) {
            // Open and read input JSON file
            fp = fopen(argv[optind++], "r");
            if (fp != NULL) {
                // Go to the end of the file
                if (fseek(fp, 0l, SEEK_END) == 0) {
                    // Determine file size
                    long bufsize = ftell(fp);
                    if (bufsize == -1) {
                        continue;
                    }

                    // Allocate our buffer to that size
                    buffer = malloc(sizeof(char) * (bufsize + 1));

                    // Go back to the start of the file
                    if (fseek(fp, 0l, SEEK_SET) != 0) {
                    }

                    // Read the entire file into memory
                    size_t newlen = fread(buffer, sizeof(char), bufsize, fp);
                    if (ferror(fp) != 0) {
                        fprintf(stderr, "Error: unable to read file %s\n", argv[optind - 1]);
                        continue;
                    } else {
                        buffer[newlen++] = '\0';
                        generate_chart(buffer);
                    }
                }
                fclose(fp);
            }

            free(buffer);
            buffer = NULL;
        }
    } else {
        fprintf(stderr, "Error: No input file(s) specified\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
