#!/bin/sh

POSTDATA=$(cat -)
if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ "$POSTDATA" = "cookie1=" ]; then
        echo "Set-Cookie: cookie1=; Max-Age=-1"
    else
        echo "Set-Cookie: $POSTDATA"
    fi
    POSTDATA="Last Posted: \"$POSTDATA\""
fi
echo Content-type: text/html
echo
cat <<END
<html>
<head>
<title>Cookie</title>
</head>
<body>
<h2>Cookie</h2>
<form method="post">
    <input type="text" name="cookie1" placeholder="Cookie value" value="">
    <input type="submit" value="POST">
</form>
$POSTDATA
<h3>ENV</h3>
<pre>
$(env)
</pre>
END
