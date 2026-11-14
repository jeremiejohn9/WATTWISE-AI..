
import express from "express";
import dotenv from "dotenv";

dotenv.config();

const app = express();
const PORT = 3000;

app.use(express.json({ limit: "10kb" }));

// Serve the website from the public folder
app.use(express.static("public"));

app.post("/api/chat", async (req, res) => {
    try {
        const { message, sensorData } = req.body;

        if (!process.env.GROQ_API_KEY) {
            return res.status(500).json({
                error: "Groq API key is missing. Check your .env file."
            });
        }

        if (typeof message !== "string" || !message.trim()) {
            return res.status(400).json({
                error: "Please enter a question."
            });
        }

        const classroomData = JSON.stringify(sensorData ?? {});

        const completion = await fetch(
            "https://api.groq.com/openai/v1/chat/completions",
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${process.env.GROQ_API_KEY}`
                },
                body: JSON.stringify({
                    model: "openai/gpt-oss-20b",
                    messages: [
                        {
                            role: "system",
                            content: `
You are WATTWISE Assistant.

You help students understand their smart classroom
energy-saving project.

You can explain:
- Classroom energy usage
- Smart lighting
- PIR motion sensors
- LDR light sensors
- Temperature and humidity
- Energy-saving suggestions
- WATTWISE dashboard graphs

Use simple, friendly language.

Current classroom sensor readings:
${classroomData}

Never invent sensor readings.
Clearly explain that energy calculations are estimates,
not direct electricity measurements.
If a reading is missing, say so.
Keep answers relevant to WATTWISE.
                            `
                        },
                        {
                            role: "user",
                            content: message.trim()
                        }
                    ],
                    temperature: 0.7,
                    max_tokens: 500
                })
            }
        );

        const result = await completion.json();

        if (!completion.ok) {
            console.error("Groq API error:", result);
            return res.status(500).json({
                error: "Groq could not generate a reply. Check your API key and model limits."
            });
        }

        const reply = result.choices?.[0]?.message?.content;

        res.json({
            reply: reply || "I couldn't generate a response."
        });

    } catch (error) {
        console.error("Server error:", error);

        res.status(500).json({
            error: "Something went wrong connecting to WATTWISE AI."
        });
    }
});

app.listen(PORT, () => {
    console.log("--------------------------------");
    console.log("       WATTWISE AI SERVER");
    console.log("--------------------------------");
    console.log(`Website: http://localhost:${PORT}`);
    console.log("AI Provider: Groq");
    console.log("--------------------------------");
});
