String.prototype.trim = function() { return this.replace(/^\s+|\s+$/, ''); };
var EmailFilter = /^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;

// 
// Make an ajax request to a server and send the response to a call back
//
function MakeAjaxRequest(method, uri, callback, data, len, headers)
{
    var httpRequest = GetHttpRequest()
    
    if (httpRequest == null)
    {
        alert("You are not ajax enabled!  This is being fixed!")
        return false;
    }

    // 
    // Called as we get a list of files to fill up on
    //
    httpRequest.onreadystatechange = function(response)
    {
        callback(httpRequest, response);
    }

    httpRequest.open(method, uri, true)

    if (headers != null)
    {
        for (var hdr in headers)
        {
            httpRequest.setRequestHeader(hdr, headers[hdr]);
        }
    }

    if (data && len)
    {
        httpRequest.setRequestHeader("Content-Length", len);
    }
    else
    {
        data = null;
    }

    httpRequest.send(data);

    return true;
}

// 
// This gets the XMLHttpRequest object for us 
// in a platform independant way
//
function GetHttpRequest()
{
    var xmlHttp;
    try
    {
        // Firefox, Opera 8.0+, Safari
        xmlHttp=new XMLHttpRequest();
    }
    catch (e)
    {
        // Internet Explorer
        try
        {
            xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
        }
        catch (e)
        {
            try
            {
                xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
            }
            catch (e)
            {
                alert("Your browser does not support AJAX!");
                return null;
            }
        }
    }

    return xmlHttp;
}

// 
// Note: This is not browser independent
//
function ElementById(name)
{
    return document.getElementById(name);
}