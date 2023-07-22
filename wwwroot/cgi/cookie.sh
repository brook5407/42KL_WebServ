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
<h3>ENVP</h3>
<pre>
$(env)
</pre>
END

# echo all arguments
echo "<h3>Arguments</h3>"
echo "<pre>"
for arg in "$@"
do
    echo "$arg"
done
echo "</pre>"

# split query string
echo "<h3>Query String</h3>"
echo "<pre>"
echo "$QUERY_STRING" | tr '&' '\n'
echo "</pre>"
echo "</body>"
echo "</html>"