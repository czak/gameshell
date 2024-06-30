#!/usr/bin/env ruby
require 'json'
require 'bigdecimal'

if ARGV.count != 2
  $stderr.puts("Usage: mkfont.rb path_to.ttf name")
  exit(1)
end

font_path = ARGV[0]
name = ARGV[1]

puts "Loading #{font_path} as #{name}..."

system(
  "msdf-atlas-gen",
  "-font", font_path,
  "-size", "64",
  "-type", "msdf",
  "-pxrange", "2",
  "-format", "text",
  "-yorigin", "top",
  "-dimensions", "512", "512",
  "-imageout", "/tmp/font.txt",
  "-json", "/tmp/font.json",
)

json = JSON.parse(File.read("/tmp/font.json"), decimal_class: BigDecimal)
pixels = File.read("/tmp/font.txt").split(/\s+/)

glyphs = json["glyphs"].map do |g|
  p = g["planeBounds"] || Hash.new(BigDecimal('0'))
  t = g["atlasBounds"] || Hash.new(BigDecimal('0'))
  adv = g["advance"]

  <<~TEXT.gsub(/\n+/, ' ').squeeze(' ').strip
    {
      .pl = #{p["left"].to_s("F")},
      .pb = #{p["bottom"].to_s("F")},
      .pr = #{p["right"].to_s("F")},
      .pt = #{p["top"].to_s("F")},
      .tl = #{t["left"].to_s("F")},
      .tb = #{t["bottom"].to_s("F")},
      .tr = #{t["right"].to_s("F")},
      .tt = #{t["top"].to_s("F")},
      .advance = #{adv.to_s("F")}
    },
  TEXT
end

texture = pixels.each_slice(16).map do |line|
  line.map { |n| "0x#{n}" }.join(", ")
end

File.write("#{name}.c", <<~OUT)
#include "font.h"

struct font font = {
	.glyphs = {
		#{glyphs.join("\n\t\t")}
	},
	.texture = {
	  #{texture.join(",\n\t\t")}
	},
};
OUT
