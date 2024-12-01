import { serve } from "https://deno.land/std/http/server.ts";

const port = 8080;

async function handler(request: Request): Promise<Response> {
  const url = new URL(request.url);
  const headers = new Headers({
    "Content-Type": "application/json",
  });

  // Helper to read and parse JSON body
  const getJsonBody = async () => {
    try {
      const text = await request.text();
      return text ? JSON.parse(text) : {};
    } catch (e) {
      console.error("Error parsing JSON:", e);
      return {};
    }
  };

  console.log(`${request.method} ${url.pathname}`);

  try {
    // Route handlers
    switch (url.pathname) {
      case "/test": {
        const body = await getJsonBody();
        const status = request.method === "POST" ? 201 : 200;
        return new Response(
          JSON.stringify({
            status: "success",
            message: `${request.method} response`,
            received: body,
          }),
          { status, headers }
        );
      }

      case "/headers": {
        // Extract headers, converting all keys to lowercase
        const receivedHeaders: Record<string, string> = {};
        request.headers.forEach((value, key) => {
          // Only include custom headers (skip standard ones)
          if (key.toLowerCase().startsWith('x-') || 
              key.toLowerCase().startsWith('another-')) {
            receivedHeaders[key.toLowerCase()] = value;
          }
        });

        return new Response(
          JSON.stringify({
            status: "success",
            headers: receivedHeaders,
          }),
          { status: 200, headers }
        );
      }

      case "/echo": {
        const body = await getJsonBody();
        return new Response(
          JSON.stringify({
            status: "success",
            received: body,
          }),
          { status: 200, headers }
        );
      }

      case "/slow": {
        await new Promise(resolve => setTimeout(resolve, 2000));
        return new Response(
          JSON.stringify({
            status: "success",
            message: "Slow response",
          }),
          { status: 200, headers }
        );
      }

      default:
        return new Response(
          JSON.stringify({
            status: "error",
            message: "Not found",
          }),
          { status: 404, headers }
        );
    }
  } catch (e) {
    console.error("Error handling request:", e);
    return new Response(
      JSON.stringify({
        status: "error",
        message: "Internal server error",
        error: e.message,
      }),
      { status: 500, headers }
    );
  }
}

console.log(`HTTP server running on http://localhost:${port}`);
await serve(handler, { port });
