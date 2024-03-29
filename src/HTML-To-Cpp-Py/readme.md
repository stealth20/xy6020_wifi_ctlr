# HTML/CSS/JS files to C++ variable convert

Simple script for minify and convert HTML/CSS/JS/SVG files to C++ variables. For example, to create a web interface in Arduino / ESP8266 / ES32 projects.

Can work offline using python libs (have to be installed by `pip install -r requirements.txt`):
* css_html_js_minify
* htmlmin

Or use API of:
* https://html-minifier.com/
* https://cssminifier.com/
* https://javascript-minifier.com/

## Usage

Move your files to `content/` folder and run script. Result will be put at `result.txt` file.

**Input file:**
```html
<a id="wlanpass"></a>
<table>
	<tr>
		<td>SSID&nbsp;</td>
		<td>
			<input type="text" name="wlanssid" id="wlanssid" placeholder="Enter name" value="" maxlength="34" />
		</td>
	</tr>
	<tr>
		<td>Password&nbsp;</td>
		<td>
			<input type="password" name="wlanpass" id="wlanpass" placeholder="Enter password" value="" maxlength="64" />
		</td>
	</tr>
</table>
<br />
```

**Result:**
```c++
// Length 304 / 365
const char html_file[] = "<a id=wlanpass></a><table><tr><td>SSID&nbsp;</td><td><input type=text name=wlanssid id=wlanssid placeholder=\"Enter name\" value maxlength=34></td></tr><tr><td>Password&nbsp;</td><td><input type=password name=wlanpass id=wlanpass placeholder=\"Enter password\" value maxlength=64></td></tr></table><br>";

```

## Configure

You can change settings by changing the variables:
```py
use_api_for_html = False # use WEB service for HTML minify
use_api_for_css = True # use WEB service for CSS minify
use_api_for_js = True # use WEB service for JS minify

split_lines = False # split output into multiple lines
escape_percent = False # escape % character to %% (for printf formatting string)

result_file = "content/result.txt" # output file
```
