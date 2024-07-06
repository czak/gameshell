#!/usr/bin/env ruby
require 'json'
require 'bigdecimal'

if ARGV.count != 2
  $stderr.puts("Usage: mkfont.rb path_to.ttf name")
  exit(1)
end

font_path = ARGV[0]
name = ARGV[1]
size = "64"
pxrange = "4"
dimensions = ["512", "512"]

system(
  "msdf-atlas-gen",
  "-font", font_path,
  "-size", size,
  "-type", "msdf",
  "-pxrange", pxrange,
  "-format", "text",
  "-yorigin", "top",
  "-dimensions", dimensions[0], dimensions[1],
  "-imageout", "/tmp/font.txt",
  "-json", "/tmp/font.json",
)

# also generate png
system(
  "msdf-atlas-gen",
  "-font", font_path,
  "-size", size,
  "-type", "msdf",
  "-pxrange", pxrange,
  "-format", "png",
  "-yorigin", "top",
  "-dimensions", dimensions[0], dimensions[1],
  "-imageout", "/tmp/font.png",
)

json = JSON.parse(File.read("/tmp/font.json"), decimal_class: BigDecimal)
pixels = File.read("/tmp/font.txt").split(/\s+/)

glyphs = json["glyphs"].map do |g|
  p = g["planeBounds"] || Hash.new(BigDecimal('0'))
  t = g["atlasBounds"] || Hash.new(BigDecimal('0'))
  adv = g["advance"]

  <<~TEXT.gsub(/\n+/, ' ').squeeze(' ').strip
    {
      .pl = #{BigDecimal(p["left"]).to_s("F")},
      .pb = #{BigDecimal(p["bottom"]).to_s("F")},
      .pr = #{BigDecimal(p["right"]).to_s("F")},
      .pt = #{BigDecimal(p["top"]).to_s("F")},
      .tl = #{BigDecimal(t["left"]).to_s("F")},
      .tb = #{BigDecimal(t["bottom"]).to_s("F")},
      .tr = #{BigDecimal(t["right"]).to_s("F")},
      .tt = #{BigDecimal(t["top"]).to_s("F")},
      .advance = #{BigDecimal(adv).to_s("F")}
    },
  TEXT
end

texture = pixels.each_slice(16).map do |line|
  line.map { |n| "0x#{n}" }.join(", ")
end

File.write("#{name}.c", <<~OUT)
#include "font.h"

// mkfont.rb #{font_path} #{name}
struct font font = {
	.glyphs = {
		#{glyphs.join("\n\t\t")}
	},
	.texture = {
	  #{texture.join(",\n\t\t")}
	},
};
OUT
