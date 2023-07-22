#!/bin/sh

POSTDATA=$(cat -)
if [ "$REQUEST_METHOD" = "POST" ]; then
    if [ "$POSTDATA" = "data=" ]; then
        echo "X-Replace-Session: "
    else
        echo "X-Replace-Session: $POSTDATA"
    fi
    POSTDATA="Last Posted: \"$POSTDATA\""
fi
cat <<END
Content-Type: text/html

<html><body>
<form method="post">
    <input type="text" name="data" placeholder="value" value="">
    <input type="submit" value="POST">
</form>
<h3>ENV</h3>
<pre>
$(env)
</pre>
</body></html>
END
