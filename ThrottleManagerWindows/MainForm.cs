using System;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Timer = System.Windows.Forms.Timer;

namespace MausManager;

public class MainForm : Form
{
    private readonly DataGridView grid = new();
    private readonly Label clockLabel = new();
    private readonly Timer clockTimer = new();
    private readonly DateTime appStart = DateTime.Now;
    private const string CsvFile = "maus.csv";

    public MainForm()
    {
        Text = "ESP32 ThrottleMaus Manager";
        Width = 1000;
        Height = 560;

        clockLabel.Dock = DockStyle.Top;
        clockLabel.Height = 42;
        clockLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
        clockLabel.Font = new System.Drawing.Font("Consolas", 16, System.Drawing.FontStyle.Bold);
        Controls.Add(clockLabel);

        grid.Dock = DockStyle.Top;
        grid.Height = 360;
        grid.AllowUserToAddRows = true;
        grid.RowHeadersVisible = false;
        grid.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
        grid.Columns.Add("IP", "IP");
        grid.Columns.Add("Color", "Culoare");
        grid.Columns.Add("Loco", "Locomotiva");
        grid.Columns.Add("Start", "TimpStart");
        grid.Columns.Add("Elapsed", "De la pornire");
        grid.CellValueChanged += (_, __) => SaveCsv();
        grid.UserDeletedRow += (_, __) => SaveCsv();
        Controls.Add(grid);

        var panel = new FlowLayoutPanel
        {
            Dock = DockStyle.Bottom,
            Height = 90,
            Padding = new Padding(10)
        };

        panel.Controls.Add(MakeButton("Lock Loco", async (_, __) => await LockLocoAsync()));
        panel.Controls.Add(MakeButton("Unlock Loco", async (_, __) => await SendSelectedAsync(_ => "<loco adr=\"*\"/>")));
        panel.Controls.Add(MakeButton("Lock Maus", async (_, __) => await SendSelectedAsync(_ => "<lock>")));
        panel.Controls.Add(MakeButton("Unlock Maus", async (_, __) => await SendSelectedAsync(_ => "<unlock>")));
        panel.Controls.Add(MakeButton("Save CSV", (_, __) => SaveCsv()));
        panel.Controls.Add(MakeButton("Reload CSV", (_, __) => { grid.Rows.Clear(); LoadCsv(); }));

        Controls.Add(panel);

        clockTimer.Interval = 1000;
        clockTimer.Tick += (_, __) => UpdateClockAndElapsed();
        clockTimer.Start();
        UpdateClockAndElapsed();

        Load += (_, __) => LoadCsv();
        FormClosing += (_, __) => SaveCsv();
    }

    private void UpdateClockAndElapsed()
    {
        clockLabel.Text = DateTime.Now.ToString("HH:mm:ss");

        foreach (DataGridViewRow row in grid.Rows)
        {
            if (row.IsNewRow)
                continue;

            var startText = row.Cells["Start"].Value?.ToString();

            if (string.IsNullOrWhiteSpace(startText))
            {
                row.Cells["Elapsed"].Value = "";
                continue;
            }

            if (TimeSpan.TryParse(startText, out TimeSpan startTime))
            {
                TimeSpan now = DateTime.Now.TimeOfDay;
                TimeSpan elapsed = now - startTime;

                // dacă trece de miezul nopții
                if (elapsed < TimeSpan.Zero)
                    elapsed += TimeSpan.FromDays(1);

                row.Cells["Elapsed"].Value = elapsed.ToString(@"hh\:mm\:ss");
            }
            else
            {
                row.Cells["Elapsed"].Value = "--:--:--";
            }
        }
    }

    private Button MakeButton(string text, EventHandler onClick)
    {
        var b = new Button
        {
            Text = text,
            Width = 160,
            Height = 40,
            Margin = new Padding(8)
        };
        b.Click += onClick;
        return b;
    }

    private async Task LockLocoAsync()
    {
        if (grid.SelectedRows.Count == 0)
        {
            MessageBox.Show("Selectează una sau mai multe linii.");
            return;
        }

        foreach (DataGridViewRow row in grid.SelectedRows)
        {
            var adr = row.Cells["Loco"].Value?.ToString()?.Trim();
            if (string.IsNullOrWhiteSpace(adr))
                continue;

            var ok = await SendToRowAsync(row, $"<loco adr=\"{adr}\"/>");
            if (ok)
                row.Cells["Start"].Value = DateTime.Now.ToString("HH:mm:ss");
        }

        SaveCsv();
    }

    private async Task SendSelectedAsync(Func<DataGridViewRow, string?> messageFactory)
    {
        if (grid.SelectedRows.Count == 0)
        {
            MessageBox.Show("Selectează una sau mai multe linii.");
            return;
        }

        foreach (DataGridViewRow row in grid.SelectedRows)
        {
            var msg = messageFactory(row);
            if (!string.IsNullOrWhiteSpace(msg))
                await SendToRowAsync(row, msg);
        }

        SaveCsv();
    }

    private async Task<bool> SendToRowAsync(DataGridViewRow row, string message)
    {
        var ip = row.Cells["IP"].Value?.ToString()?.Trim();
        if (string.IsNullOrWhiteSpace(ip))
            return false;

        try
        {
            using var client = new TcpClient();
            await client.ConnectAsync(ip, 8983);
            using var stream = client.GetStream();
            var data = Encoding.ASCII.GetBytes(message + "\\n");
            await stream.WriteAsync(data, 0, data.Length);
            return true;
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Eroare la {ip}: {ex.Message}");
            return false;
        }
    }

    private void LoadCsv()
    {
        if (!File.Exists(CsvFile))
            return;

        foreach (var line in File.ReadLines(CsvFile).Skip(1))
        {
            if (string.IsNullOrWhiteSpace(line))
                continue;

            var p = line.Split(',');
            while (p.Length < 5)
                p = p.Append("").ToArray();

            grid.Rows.Add(p[0], p[1], p[2], p[3], p[4]);
        }
    }

    private void SaveCsv()
    {
        using var sw = new StreamWriter(CsvFile, false, Encoding.UTF8);
        sw.WriteLine("IP,Culoare,Locomotiva,TimpStart,DeLaPornire");

        foreach (DataGridViewRow row in grid.Rows)
        {
            if (row.IsNewRow)
                continue;

            string V(int i) => row.Cells[i].Value?.ToString()?.Replace(",", " ") ?? "";

            sw.WriteLine($"{V(0)},{V(1)},{V(2)},{V(3)},{V(4)}");
        }
    }
}
