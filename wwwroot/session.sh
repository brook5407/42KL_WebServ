#!/bin/sh

POSTDATA=$(cat -)
if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ "$POSTDATA" = "key1=" ]; then
        echo "X-Replace-Session: "
    else
        echo "X-Replace-Session: $POSTDATA"
    fi
    POSTDATA="Last Posted: \"$POSTDATA\""
fi
echo Content-type: text/html
echo
cat <<END
<html>
<head>
<title>Session</title>
</head>
<body>
<h2>Session</h2>
<form method="post">
    <input type="text" name="key1" placeholder="value" value="">
    <input type="submit" value="POST">
</form>
$POSTDATA
<h3>ENV</h3>
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