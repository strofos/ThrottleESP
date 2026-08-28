package com.example.mausmanager

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject
import java.io.File
import java.net.InetSocketAddress
import java.net.Socket
import java.text.SimpleDateFormat
import java.util.*

data class MausConfig(val ip: String, val color: String, val locomotive: String)
data class MausRuntime(var startTime: Long? = null, var locked: Boolean = false, var status: String = "")

class MausStorage(private val file: File) {
    fun save(list: List<MausConfig>) {
        val a = JSONArray()
        list.forEach { m ->
            a.put(JSONObject().apply {
                put("ip", m.ip)
                put("color", m.color)
                put("locomotive", m.locomotive)
            })
        }
        file.writeText(a.toString(2))
    }

    fun load(): List<MausConfig> {
        if (!file.exists()) return emptyList()
        return try {
            val a = JSONArray(file.readText())
            List(a.length()) { i ->
                val o = a.getJSONObject(i)
                MausConfig(o.getString("ip"), o.getString("color"), o.getString("locomotive"))
            }
        } catch (_: Exception) {
            emptyList()
        }
    }
}

class MainActivity : ComponentActivity() {
    override fun onCreate(b: Bundle?) {
        super.onCreate(b)
        setContent {
            MausManagerApp(MausStorage(File(filesDir, "mauses.json")))
        }
    }
}

@Composable
fun MausManagerApp(storage: MausStorage) {
    var mauses by remember { mutableStateOf(storage.load()) }
    val runtime = remember { mutableStateMapOf<String, MausRuntime>() }
    var now by remember { mutableLongStateOf(System.currentTimeMillis()) }
    var selected by remember { mutableIntStateOf(-1) }
    var add by remember { mutableStateOf(false) }
    var del by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    LaunchedEffect(Unit) {
        while (true) {
            now = System.currentTimeMillis()
            delay(1000)
        }
    }

    MaterialTheme {
        Column(
            Modifier
                .fillMaxSize()
                .padding(12.dp)
                .navigationBarsPadding()
                .padding(bottom = 8.dp)
        ) {
            Card(
                Modifier.fillMaxWidth(),
                RoundedCornerShape(16.dp)
            ) {
                Box(
                    Modifier
                        .fillMaxWidth()
                        .padding(18.dp),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        formatTime(now),
                        fontSize = 34.sp,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
            Spacer(Modifier.height(10.dp))
            LazyColumn(Modifier.weight(1f)) {
                itemsIndexed(mauses, key = { _, m -> m.ip }) { i, m ->
                    val st = runtime.getOrPut(m.ip) { MausRuntime() }
                    MausCard(
                        m,
                        st,
                        now,
                        selected == i,
                        onSelect = { selected = i }
                    ) { cmd, action ->
                        scope.launch {
                            st.status = "Conectare..."
                            if (sendTcp(m.ip, cmd)) {
                                action(st)
                                st.status = "OK"
                            } else {
                                st.status = "Eroare TCP"
                            }
                            runtime[m.ip] = st
                        }
                    }
                    Spacer(Modifier.height(10.dp))
                }
            }
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = { add = true },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("+ ADAUGĂ MAUS")
                }
                Button(
                    onClick = { del = true },
                    modifier = Modifier.weight(1f),
                    enabled = selected in mauses.indices
                ) {
                    Text("− ȘTERGE MAUS")
                }
            }
        }
    }

    if (add) {
        AddMausDialog(
            onCancel = { add = false },
            onAdd = { ip, c, l ->
                if (mauses.none { it.ip == ip }) {
                    mauses = mauses + MausConfig(ip, c, l)
                    storage.save(mauses)
                }
                add = false
            }
        )
    }

    if (del && selected in mauses.indices) {
        val m = mauses[selected]
        AlertDialog(
            onDismissRequest = { del = false },
            title = { Text("Ștergere Maus") },
            text = { Text("Ștergi ${m.ip} / locomotiva ${m.locomotive}?") },
            confirmButton = {
                TextButton(onClick = {
                    runtime.remove(m.ip)
                    mauses = mauses.toMutableList().apply { removeAt(selected) }
                    storage.save(mauses)
                    selected = -1
                    del = false
                }) {
                    Text("ȘTERGE")
                }
            },
            dismissButton = {
                TextButton(onClick = { del = false }) {
                    Text("ANULEAZĂ")
                }
            }
        )
    }
}

@Composable
fun MausCard(
    m: MausConfig,
    s: MausRuntime,
    now: Long,
    selected: Boolean,
    onSelect: () -> Unit,
    onCommand: (String, (MausRuntime) -> Unit) -> Unit
) {
    val elapsed = s.startTime?.let { now - it }
    Card(
        Modifier
            .fillMaxWidth()
            .clickable { onSelect() },
        RoundedCornerShape(16.dp),
        colors = CardDefaults.cardColors(
            if (selected) MaterialTheme.colorScheme.secondaryContainer
            else MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(Modifier.padding(14.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Box(
                    Modifier
                        .size(22.dp)
                        .background(colorFromName(m.color), CircleShape)
                )
                Spacer(Modifier.width(10.dp))
                Text(
                    "MAUS",
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold
                )
                Spacer(Modifier.weight(1f))
                Text(if (s.locked) "LOCKED" else "READY")
            }
            Text("IP: ${m.ip}")
            Text("Locomotiva: ${m.locomotive}")
            Text("TimpStart: ${s.startTime?.let(::formatTime) ?: "-"}")
            Text(
                "De la pornire: ${elapsed?.let(::formatElapsed) ?: "-"}",
                fontSize = 18.sp
            )
            if (s.status.isNotBlank()) Text(s.status)
            Spacer(Modifier.height(8.dp))
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                Button(
                    onClick = {
                        onCommand("<loco adr=\"${m.locomotive}\"/>") {
                            it.startTime = System.currentTimeMillis()
                            it.locked = true
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Lock Loco")
                }
                Button(
                    onClick = {
                        onCommand("<loco adr=\"*\"/>") {
                            it.startTime = null
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Unlock Loco")
                }
            }
            Spacer(Modifier.height(6.dp))
            Row(
                Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                Button(
                    onClick = {
                        onCommand("<lock>") {
                            it.locked = true
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Lock Maus")
                }
                Button(
                    onClick = {
                        onCommand("<unlock>") {
                            it.locked = false
                        }
                    },
                    modifier = Modifier.weight(1f)
                ) {
                    Text("Unlock Maus")
                }
            }
        }
    }
}

@Composable
fun AddMausDialog(
    onCancel: () -> Unit,
    onAdd: (String, String, String) -> Unit
) {
    var ip by remember { mutableStateOf("") }
    var color by remember { mutableStateOf("") }
    var loco by remember { mutableStateOf("") }

    AlertDialog(
        onDismissRequest = onCancel,
        title = { Text("Adaugă Maus") },
        text = {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedTextField(
                    ip,
                    { ip = it },
                    label = { Text("IP") },
                    singleLine = true
                )
                OutlinedTextField(
                    color,
                    { color = it },
                    label = { Text("Culoare") },
                    singleLine = true
                )
                OutlinedTextField(
                    loco,
                    { loco = it },
                    label = { Text("Locomotiva") },
                    singleLine = true
                )
            }
        },
        confirmButton = {
            TextButton(
                enabled = ip.isNotBlank() && loco.isNotBlank(),
                onClick = { onAdd(ip.trim(), color.trim(), loco.trim()) }
            ) {
                Text("ADAUGĂ")
            }
        },
        dismissButton = {
            TextButton(onClick = onCancel) {
                Text("ANULEAZĂ")
            }
        }
    )
}

suspend fun sendTcp(ip: String, cmd: String) = withContext(Dispatchers.IO) {
    try {
        Socket().use { s ->
            s.connect(InetSocketAddress(ip, 8983), 2000)
            s.getOutputStream().use { o ->
                o.write((cmd + "\n").toByteArray(Charsets.US_ASCII))
                o.flush()
            }
        }
        true
    } catch (_: Exception) {
        false
    }
}

fun formatTime(ms: Long) = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date(ms))

fun formatElapsed(ms: Long): String {
    var s = ms / 1000
    val h = s / 3600
    s %= 3600
    val m = s / 60
    s %= 60
    return String.format(Locale.getDefault(), "%02d:%02d:%02d", h, m, s)
}

fun colorFromName(n: String) = when (n.lowercase(Locale.getDefault())) {
    "roșu", "rosu", "red" -> Color.Red
    "verde", "green" -> Color.Green
    "albastru", "blue" -> Color.Blue
    "galben", "yellow" -> Color.Yellow
    "portocaliu", "orange" -> Color(0xFFFF9800)
    "mov", "purple" -> Color(0xFF9C27B0)
    else -> Color.Gray
}
